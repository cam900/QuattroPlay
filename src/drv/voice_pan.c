/*
    Quattro - Volume & panning functions
*/
#include "quattro.h"
#include "voice.h"
#include "helper.h"
#include "tables.h"

// In the code for most sound drivers there are alternate versions of
// Q_VoicePanConvert and Q_VoicePosConvert.
//
// The alternate version of Q_VoicePanConvert is mostly identical to the
// normal function, except it reads from a pan conversion table in RAM written
// by track command 0x2e, which is also supposed to enable the function.
// It's not implemented as I have not seen it used anywhere yet.
//
// The alternate version of Q_VoicePosConvert uses the ROM pan conversion
// table and keeps the final amplitude constant, unlike the normal function
// which uses a linear conversion and attenuates one side without boosting
// the other. The track command that enables it (0x2c) is broken in later
// sound driver versions.

// call 0x2c - convert the 8-bit signed pan value to C352 volume register offsets
// source: 0x6a1c (NOTE: unimplemented "alt" function at 0x69ce)
void Q_VoicePanConvert(Q_State *Q,int8_t pan,uint16_t *VolumeF,uint16_t *VolumeR)
{
    uint8_t FL,FR,RL,RR;
    FL=FR=RL=RR=0xff;

    uint8_t offset = (pan+0x20)&Q->PanMask;
    uint8_t panL = Q_PanTable[offset&0x3f];
    uint8_t panR = Q_PanTable[(offset^0x3f)&0x3f];

    if(offset<0x40)
    {
        FR=panR;
        FL=panL;
    }
    else if(offset<0x80)
    {
        RR=panR;
        FR=panL;
    }
    else if(offset<0xc0)
    {
        RL=panR;
        RR=panL;
    }
    else
    {
        FL=panR;
        RL=panL;
    }

    *VolumeF = (FL<<8)|(FR&0xff);
    *VolumeR = (RL<<8)|(RR&0xff);
}

// call 0x2e - convert the 8-bit position values to C352 volume register offsets
// source: 0x691a (NOTE: unimplemented "alt" function at 0x6824)
void Q_VoicePosConvert(Q_State* Q,int8_t xpos,int8_t ypos,uint16_t *VolumeF,uint16_t *VolumeR)
{
    if(!Q->PanMask)
        return Q_VoicePanConvert(Q,0,VolumeF,VolumeR);

    uint16_t FL,FR,RL,RR;
    FL=FR=RL=RR=0;
    uint8_t offset;

    if(xpos < 0)
    {
        // attenuate right channel
        offset = xpos ^ 0xff;
        FR=RR=offset<<2;
    }
    else
    {
        // attenuate left channel
        offset = xpos;
        FL=RL=offset<<2;
    }

    if(ypos <0)
    {
        // attenuate front speakers
        offset = (ypos ^ 0xff)<<2;
        FL += offset;
        if(FL > 0xff)
            FL = 0xff;
        FR += offset;
        if(FR > 0xff)
            FR = 0xff;
    }
    else
    {
        // attenuate rear speakers
        offset = (ypos)<<2;
        RL += offset;
        if(RL > 0xff)
            RL = 0xff;
        RR += offset;
        if(RR > 0xff)
            RR = 0xff;
    }

    *VolumeF = (FL<<8)|(FR&0xff);
    *VolumeR = (RL<<8)|(RR&0xff);
}

// set panning (split from Q_VoiceKeyOn)
// source: 0x6586
void Q_VoicePanSet(Q_State *Q,int VoiceNo,Q_Voice* V)
{
    uint32_t pos;
    uint8_t d;

    // spaghetti to untangle at 0x6586
    switch(V->PanMode)
    {
    case Q_PANMODE_IMM:
        Q_VoicePanConvert(Q,V->Pan,&V->VolumeFront,&V->VolumeRear);
        V->PanState = Q_PAN_SET;
        break;
    case Q_PANMODE_REG:
        V->PanSource = &Q->Register[V->Pan];
        V->PanState = Q_PAN_REG;
        break;
    default: // POS
        if(V->PanMode >= 0x40 && V->PanMode < 0x80)
            break;
        Q_VoicePosConvert(Q,V->Pan,V->PanMode,&V->VolumeFront,&V->VolumeRear);
        V->PanState = Q_PAN_SET;
        break;
    case Q_PANMODE_POSREG:
        V->PanSource = &Q->Register[V->Pan];
        V->PanState = Q_PAN_POSREG;
        break;
    case Q_PANMODE_POSENV:
    case Q_PANMODE_POSENV_SET:
        pos = Q->McuDataPosBase;
        pos += Q_ReadWord(Q, Q->TableOffset[Q_TOFFSET_PANTABLE]+(((V->Pan-1)&0xff)*2));
        //Q_DEBUG("V=%02x position envelope (id %02x at %06x)\n",VoiceNo,V->Pan,pos);
        pos++;
        // Position envelopes aren't supported yet, just read the initial value for now.
        Q_VoicePosConvert(Q,Q_ReadByte(Q,pos+1),Q_ReadByte(Q,pos),&V->VolumeFront,&V->VolumeRear);
        V->PanState = Q_PAN_SET;
        break;
    case Q_PANMODE_ENV:
    case Q_PANMODE_ENV_SET:
        pos = Q->McuDataPosBase;

        // dirtdash song 0x26 sets pan envelope id 0x00, which is invalid (index = 0xff)
        // causing snare drums (and some other instruments) to be almost muted.
        // i assume the intended effect was to disable the pan envelope, but this does not
        // happen (since the bits in the WriteChannel mask are set for these channels)
        pos += Q_ReadWord(Q, Q->TableOffset[Q_TOFFSET_PANTABLE]+(((V->Pan-1)&0xff)*2));
        d = Q_ReadByte(Q,pos++);
        if(d == 0)
        {
            // always reset
            V->PanEnvDelay = 1;
        }
        else if(d == 0xff)
        {
            // later drivers: reset if pan envelope command was retriggered.
            if(Q->McuVer < Q_MCUVER_Q00 || (Q->McuVer >= Q_MCUVER_Q00 && V->PanMode != Q_PANMODE_ENV_SET && V->PanMode != Q_PANMODE_POSENV_SET))
            {
                // always reset on new pan envelope
                if(V->PanUpdateFlag == V->Pan)
                    break;
            }

            if(V->PanMode == Q_PANMODE_ENV_SET)
                V->PanMode = Q_PANMODE_ENV;
            else if(V->PanMode == Q_PANMODE_POSENV_SET)
                V->PanMode = Q_PANMODE_POSENV;
            V->Channel->PanMode = V->PanMode2 = V->PanMode;

            V->PanEnvDelay = 1;
        }
        else
            V->PanEnvDelay = d;

        d = Q_ReadByte(Q,pos++);
        V->PanEnvTarget = d<<8;
        V->PanEnvValue = d;

        if(V->PanMode == Q_PANMODE_ENV_SET || V->PanMode == Q_PANMODE_ENV)
        {
            V->PanState = Q_PAN_ENV;
        }
        else
        {
            V->PanEnvValue2 = Q_ReadByte(Q,pos++)<<8;
            V->PanState = Q_PAN_POSENV;
        }
        V->PanEnvPos = pos;
        V->PanEnvLoop = pos;
        break;
    }

    V->PanUpdateFlag=V->Pan;
}

// write C352 volume
// source: 0x6c50
void Q_VoiceSetVolume(Q_State* Q,int VoiceNo,Q_Voice* V, uint16_t VolumeF, uint16_t VolumeR)
{
    //C352_Voice *CV = &Q->Chip.v[VoiceNo];

    uint8_t CFL,CFR,CRL,CRR;
    CFL=CFR=CRL=CRR=0;

    uint16_t FL,FR,RL,RR;
    FL=FR=RL=RR=V->VolumeMod;
    FL += VolumeF>>8;
    FR += VolumeF&0xff;
    RL += VolumeR>>8;
    RR += VolumeR&0xff;

    // convert volume
    if(FL<0x100)
        CFL=Q_VolumeTable[FL];
    if(FR<0x100)
        CFR=Q_VolumeTable[FR];
    if(RL<0x100)
        CRL=Q_VolumeTable[RL];
    if(RR<0x100)
        CRR=Q_VolumeTable[RR];

    // write to chip
    Q_C352_W(Q,VoiceNo,C352_VOL_FRONT,(CFL<<8)|(CFR&0xff));
    Q_C352_W(Q,VoiceNo,C352_VOL_REAR, (CRL<<8)|(CRR&0xff));
}

// write volume (for pan envelopes)
// source: 0x752c
void Q_VoicePanSetVolume(Q_State* Q,int VoiceNo,Q_Voice* V,int8_t pan)
{
    Q_VoicePanConvert(Q,pan,&V->VolumeFront,&V->VolumeRear);
    Q_VoiceSetVolume(Q,VoiceNo,V,V->VolumeFront,V->VolumeRear);
}

// call 0x1c - update voice panning & volume
// source: 0x71f6
// vectors: see below
void Q_VoicePanUpdate(Q_State *Q,int VoiceNo,Q_Voice* V)
{
    uint16_t VolumeF;
    uint16_t VolumeR;
    uint16_t val;
    //....
    switch(V->PanState)
    {
    case Q_PAN_SET: // 0x6c40
        Q_VoiceSetVolume(Q,VoiceNo,V,V->VolumeFront,V->VolumeRear);
        break;
    case Q_PAN_REG: // 0x7244
        Q_VoicePanConvert(Q,*V->PanSource,&VolumeF,&VolumeR);
        Q_VoiceSetVolume(Q,VoiceNo,V,VolumeF,VolumeR);
        break;
    case Q_PAN_POSREG: // 0x7204
        val = *V->PanSource;
        Q_VoicePosConvert(Q,val&0xff,val>>8,&VolumeF,&VolumeR);
        Q_VoiceSetVolume(Q,VoiceNo,V,VolumeF,VolumeR);
        break;
    case Q_PAN_ENV: // 0x727e
    case Q_PAN_ENV_LEFT: // 0x73aa
    case Q_PAN_ENV_RIGHT: // 0x7394
        Q_VoicePanEnvUpdate(Q,VoiceNo,V);
        break;
    case Q_PAN_POSENV: // 0x73ca, 0x75b0
        break;
    }
}

// update pan envelope
// source: 0x727e (first part)
void Q_VoicePanEnvUpdate(Q_State* Q,int VoiceNo,Q_Voice* V)
{
    if(V->PanEnvDelay)
    {
        V->PanEnvDelay--;
    }
    else
    {
        int32_t p;
        switch(V->PanState)
        {
        case Q_PAN_ENV:
            Q_VoicePanEnvRead(Q,VoiceNo,V);
            break;
        case Q_PAN_ENV_LEFT:
            p = V->PanEnvValue + V->PanEnvDelta;
            V->PanEnvValue=p;
            if(p>0xffff)
                Q_VoicePanEnvRead(Q,VoiceNo,V);
            break;
        case Q_PAN_ENV_RIGHT:
            p = V->PanEnvValue - V->PanEnvDelta;
            V->PanEnvValue=p;
            if(p<0)
                Q_VoicePanEnvRead(Q,VoiceNo,V);
        default:
            break;
        }
        // we got end of envelope
        if(V->PanState == Q_PAN_SET)
            return;
    }
    return Q_VoicePanSetVolume(Q,VoiceNo,V,(V->PanEnvTarget-V->PanEnvValue)>>8);
}

// read data from pan envelope and update state accordingly
// source: 0x727e (second part)
void Q_VoicePanEnvRead(Q_State* Q,int VoiceNo,Q_Voice* V)
{
    V->PanEnvValue= 0x8000|(V->PanEnvValue&0xff);

    uint8_t d,e;

    while(1)
    {
        d = Q_ReadByte(Q,V->PanEnvPos++);

        switch(d)
        {
        case 0x80: // end envelope
            V->PanState=Q_PAN_SET;
            return;
        case 0x81: // continue to next envelope and set new loop position
            V->PanEnvPos+=2;
            V->PanEnvLoop=V->PanEnvPos;
            break;
        case 0x82: // jump to loop position
            V->PanEnvPos=V->PanEnvLoop;
            break;
        case 0x83: // read next envelope and read params
            V->Channel->Pan++;
            V->Pan = V->PanUpdateFlag+1;
            e = Q_ReadByte(Q,V->PanEnvPos++);
            if(e!=0xff)
            {
                V->PanMode = Q_PANMODE_ENV_SET;
                V->PanEnvDelay = e;
            }
        case 0x84: // jump to loop and read data
            if(d==0x84)
            {
                V->PanEnvDelay=0;
                V->PanEnvPos = V->PanEnvLoop-1;
            }
            e = Q_ReadByte(Q,V->PanEnvPos++);
            V->PanEnvValue=(V->PanEnvValue&0xff00)|e;
            V->PanEnvTarget=e<<8;
            V->PanEnvLoop=V->PanEnvPos;
            break;
        default:
            e = Q_ReadByte(Q,V->PanEnvPos++);
            V->PanEnvValue = ((e<<8) - V->PanEnvTarget);
            V->PanEnvTarget = e<<8;

            // machbrkr song 23d has a weird pan envelope ('left' slide with positive delta)
            // this happens on the original sound driver but also doesn't (see Q_VoicePanSet)
            // Uncommenting below code 'fixes' the pan envelope, but will break other songs.
            if(d&0x80)
            {
                // left slide
                V->PanEnvDelta = Q_EnvelopeRateTable[-d &0xff];
                V->PanState = Q_PAN_ENV_LEFT;
#if 0
                printf("%02x = L %04x\n", VoiceNo, ~V->PanEnvValue & 0xffff);
                if(~V->PanEnvValue > 0x8000)
                    V->PanEnvValue = V->PanEnvTarget;
#endif
            }
            else
            {
                // right slide
                V->PanEnvDelta = Q_EnvelopeRateTable[d];
                V->PanState = Q_PAN_ENV_RIGHT;
#if 0
                printf("%02x = R %04x\n", VoiceNo, V->PanEnvValue & 0xffff);
                if(V->PanEnvValue > 0x8000)
                    V->PanEnvValue = V->PanEnvTarget;
#endif
            }
            return;
        }
    }
}
