#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Jan 26 15:58:24 2018

@author: slacour
"""

import time
from scipy.signal import welch
from scipy.signal import fftconvolve
import socket
from numpy import *
from struct import *
import zmq
import requests
from threading import Thread, RLock



# VALIDATION CRC
def validate_CRC(frameRawHex):
    toCheckBits=(unpackbits(frameRawHex[:-2,None],axis=1)[:,::-1].ravel()).astype('bool')
    G=array([0,0,0,1, 0,0,0,0, 0,0,1,0, 0,0,0,1],dtype=bool)
    r=16
    GFlip=G[::-1]
    N_bits=len(toCheckBits)
    SR=ones(r,dtype=bool)

    for b in toCheckBits:
        Outbit=SR[r-1]
        SR=roll(SR,1)
        SR[0]=False
        XORMask=(b!=Outbit)&GFlip
        SR= (SR !=XORMask)

    crcBits=unpackbits(array(frameRawHex[-2:][::-1],dtype=uint8))
    crcCheck=all(invert(SR)==crcBits)

    return crcCheck,crcBits


def send_init_packet():
    time.sleep(10)
    frameRawHex=array([160,146,134,166,130,168,224,160,146,134,166,130,168,101,3,240,9,82,211,114,0,34,68,148,0,244,106,80,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,59,160,200,204,186,208,99,149,59,90,231,64,196,106], dtype=uint8)
    crc=unpackbits(frameRawHex[-2:]).reshape(2,8)[:,::-1].reshape(16)
    crcCheck,crcBits=validate_CRC(frameRawHex)
    if (crcCheck == True):
        printzmq("Test: Sending initialisation packet")
        datazmq(frameRawHex)
    printzmq("Test: Sending 100Hz offset frequency")
    changeFreqzmq("REL",str(100))


def resetFreq(socket2,decode):
    time.sleep(5)
    freq=int(435.525*1e6)
    changeFreqzmq("ABS",str(freq))
    while True:
        time.sleep(2000)
        if (time.time()-decode.lastGoodFitTime>1500):
            decode.bitRateKnown=False
            changeFreqzmq("ABS",str(freq))

def changeFreq(lastFrequencyShift,limit,lastFreqChange,detection):
    if detection:
        presentTime=time.time()
        if (abs(lastFrequencyShift) > limit)&(presentTime-lastFreqChange> 30) :
                changeFreqzmq("REL",str(int(lastFrequencyShift)))
                lastFreqChange = presentTime

    return lastFreqChange

def process_data(dataList,nDataFitStart,nDataFitEnd,lastFrequencyShift,lastBitRate,lastGoodFitTime,goFast,crcList):
            startTime=time.time()
            data=hstack(dataList)
            nData=len(data)
#            print(nData)

            nDataFitZero=nDataFitStart
            nDataFit=nDataFitEnd-nDataFitZero
            dataFit=data[nDataFitZero:nDataFitZero+nDataFit]

            if goFast==False:
                N_fine=200
                bitRate=lastBitRate # bitrate
            else:
                bitRate=lastBitRate # bitrate
                N_fine=21


            Fe=38400 #frequence echantillonage
            Nsample=int(Fe/bitRate+0.5)
            Fan = array([-1,1,1,1,1,1,1,-1])# fanion RX25


            reqWelch2,fftDataWelch2=welch(dataFit,fs=Fe,return_onesided=False,nperseg=1024)
            r2=append(fft.fftshift(reqWelch2),fft.fftshift(reqWelch2))
            f2=append(fft.fftshift(fftDataWelch2),fft.fftshift(fftDataWelch2))
            freqOffset=r2[convolve(f2,ones(1024//(Nsample)),mode="same").argmax()]

#            print("..%i..with delay = %i ms"%(1,int((time.time()-startTime)*1000)))

            # FREQUENCY SMOOTHING with sync
            dataCorrected=data*exp(-1j*2*pi*freqOffset*arange(nData)/Fe)
            dataCorrected=convolve(ones(Nsample),dataCorrected,mode="same")
            dataCorrectedFit=dataCorrected[nDataFitZero:nDataFitZero+nDataFit]

            # CORRECTION FINE PHASE

            N1=32
            N2=N1*2
            NWave=int((len(dataCorrectedFit)-N2+N1)/N1)
            dataCorrectedWave=array([dataCorrectedFit[i*N1:i*N1+N2] for i in range(NWave)])

            x=real(dataCorrectedWave)
            y=imag(dataCorrectedWave)
            theta=linspace(0,pi,10,endpoint=False)
            vis=abs(x[:,:,None]*cos(theta)+y[:,:,None]*sin(theta)).sum(axis=1)
            amp_max=vis.argmax(axis=1)
            amp=array([vis[i,amp_max[i]] for i in range(len(amp_max))])
            phase=theta[amp_max]
            phase1=unwrap(phase*2)*0.5
            r=polyfit(arange(len(amp_max)),phase1,2,w=amp)
            freqOffset+=r[1]/N1/(2*pi)*Fe

            phase2=poly1d(r)(arange(len(amp_max)))
            X2=x*cos(phase2[:,None])+y*sin(phase2[:,None])
            Y2=-x*sin(phase2[:,None])+y*cos(phase2[:,None])

            phase3=poly1d(r)( (arange(len(dataCorrected))-0.5*N2-nDataFitZero) /N1)
            X=real(dataCorrected)*cos(phase3)+imag(dataCorrected)*sin(phase3)
            Y=-real(dataCorrected)*sin(phase3)+imag(dataCorrected)*cos(phase3)

            dataFineCorrected=X+1j*Y

#            print("..%i..with delay = %i ms"%(3,int((time.time()-startTime)*1000)))


            # INTERPOLATION HORLOGES
            xp=arange(nData)-nDataFitZero-nDataFit/2+0.5
            Nbit=int(nDataFit*bitRate/Fe*(1-1e-4)-10)
            xFit=(arange(Nbit)-Nbit/2+0.5)*(Fe/bitRate)
            N=int(nData*bitRate/Fe*(1-1e-4)+20)
            x=(arange(2*N)-N-0.5+Nbit/2-Nbit//2)*(Fe/bitRate)
            x=x[(x>xp.min())&(x<xp.max())]

            shiftBiteRate1=linspace(-0.5,0.5,10)*Nsample
            freqBiteRate1=linspace(-0.5e-4*N_fine,0.5e-4*N_fine,N_fine)+1

            tab=arange(nData)[(xp*min(freqBiteRate1)>min(xFit)+min(shiftBiteRate1))&(xp*min(freqBiteRate1)<max(xFit)+max(shiftBiteRate1))]
            xmin=min(tab)
            xmax=max(tab)
            if xmin >0: xmin-=1
            if xmax <nData-1: xmax+=1
            gc=array([[interp(xFit+s,xp[xmin:xmax]*f,real(dataFineCorrected[xmin:xmax]))+1j*interp(xFit+s,xp[xmin:xmax]*f,imag(dataFineCorrected[xmin:xmax])) for s in shiftBiteRate1] for f in freqBiteRate1])
            gc+=abs(real(gc))-real(gc)
            energy1=abs(gc.sum(axis=2))
            indexes=unravel_index(energy1.argmax(), energy1.shape)

#            print("..%i..with delay = %i ms"%(4,int((time.time()-startTime)*1000)))

            shiftBiteRate2=linspace(-0.1,0.1,10)*Nsample+shiftBiteRate1[indexes[1]]
            freqBiteRate2=linspace(-1.5e-4,1.5e-4,37)+freqBiteRate1[indexes[0]]

            tab=arange(nData)[(xp*min(freqBiteRate2)>min(xFit)+min(shiftBiteRate2))&(xp*min(freqBiteRate2)<max(xFit)+max(shiftBiteRate2))]
            xmin=min(tab)
            xmax=max(tab)
            if xmin >0: xmin-=1
            if xmax <nData-1: xmax+=1
            gc=array([[interp(xFit+s,xp[xmin:xmax]*f,real(dataFineCorrected[xmin:xmax]))+1j*interp(xFit+s,xp[xmin:xmax]*f,imag(dataFineCorrected[xmin:xmax])) for s in shiftBiteRate2] for f in freqBiteRate2])
            gc+=abs(real(gc))-real(gc)
            energy2=abs(gc.sum(axis=2))
            indexes=unravel_index(energy2.argmax(), energy2.shape)
            dataFinal=interp(x+shiftBiteRate2[indexes[1]],xp*freqBiteRate2[indexes[0]],real(dataFineCorrected))
            freqBiteRateFinal=freqBiteRate2[indexes[0]]

#            print("..%i..with delay = %i ms"%(5,int((time.time()-startTime)*1000)))

            #DESCRAMBLING
            boolFinal=dataFinal>0
            dataBits=((boolFinal[17:]!=boolFinal[:-17])!=boolFinal[17-12:-12])
            dataBits=(2*dataBits[1:]-1)*(2*dataBits[:-1]-1)


            # RECUPERATION FANIONS
            convFan=convolve(dataBits,Fan,mode="same")
            Fanion=where(convFan==8)[0]
            startFrame=Fanion[where(diff(Fanion)>175)[0]]+3
            endFrame=Fanion[where(diff(Fanion)>175)[0]+1]-4


            frameChecking=[]
            frameCategory=[]
            oneGoodFrame=False
            for (s,e) in zip(startFrame,endFrame):


                # DESTUFFING
                frameRaw=dataBits[s:e]
                destuff=ones(len(frameRaw),dtype=bool)
                destuff[:5]=False
                for i in range(5): destuff[1+i:]&=(frameRaw[:-1-i]>0)

                # RECUPERE INFO
                frameRaw=frameRaw[invert(destuff)]
                frameRaw=frameRaw[1:1+8*((len(frameRaw)-1)//8)]
                frameRawbit=(frameRaw>0).reshape(len(frameRaw)//8,8)

                frameRawHex=packbits(frameRawbit[:,::-1])
                crc=packbits(frameRawbit[-2:,:])

                # VALIDATION
                if len(frameRawHex) > 18:

                    callSignName=(frameRawHex[:14]>>1).tostring()
                    categoryNumber=frameRawHex[17]%32

                    if callSignName[:6]==b'PICSAT':
                        crcCheck,crcBits=validate_CRC(frameRawHex)
                        if (crcCheck == False):
                            printzmq("Corrupted CRC : "+str(callSignName))
                    else:
                        crcCheck=False
                else:
                    crcCheck=False

                frameChecking.append(crcCheck)


                if (crcCheck == True):
                    dataSave=True
                    frameCategory.append(categoryNumber)
                    if len(crcList) !=0:
                        if alltrue(array(crcList==crcBits),axis=1).max() == True:
                            print("Already checked Data from "+str(callSignName)+" -> cat %i"%categoryNumber)
                            dataSave = False
                            frameChecking[-1]+=1
                    if dataSave:
                        printzmq("Checked Data from "+str(callSignName)+" -> cat %i"%categoryNumber)
                        crcList.append(crcBits)
                        datazmq(frameRawHex)

                    lastGoodFitTime=time.time()
                    lastFrequencyShift=freqOffset
                    lastBitRate=bitRate*freqBiteRateFinal
                    oneGoodFrame=True


            print("Processed data in%5i ms with bitrate %i."%(int((time.time()-startTime)*1000),int(lastBitRate)))


            return (dataFinal,crcList,(phase1,phase2),(startTime,lastFrequencyShift,lastBitRate,oneGoodFrame,lastGoodFitTime),(startFrame,endFrame,frameChecking,frameCategory))



class Decode(Thread):
    def __init__(self):
        self.dataReady=False
        self.bitRateKnown=False
        self.lastFrequencyShift=0
        self.lastGoodFitTime=0
        self.lastFreqChange=0
        self.lastBitRate=9600
        self.goFast=False
        self.dataList=[]
        self.crcList=[]
        self.d=[]
        self.paramList=array([[0,0,0,0,0]])
        self.nDataFitStart=0
        self.nDataFitEnd=0
        Thread.__init__(self)

    def run(self):

        crcList=self.crcList
        dataList=self.dataList
        nDataFitStart=self.nDataFitStart
        nDataFitEnd=self.nDataFitEnd
        lastFrequencyShift=self.lastFrequencyShift
        lastBitRate=self.lastBitRate
        lastGoodFitTime=self.lastGoodFitTime
        goFast=self.goFast
        crcList=self.crcList
        lastFreqChange=self.lastFreqChange

        currentTime=time.time()
        if (currentTime-lastGoodFitTime) < 35:
            goFast=True
        else:
            goFast=False

        if (currentTime-lastFreqChange) < 5:
            goFast=False

        if goFast ==True:
            print("-- Processing Data from point%7i to%7i, with fast speed:"%(nDataFitStart,nDataFitEnd))
        else:
            print("-- Processing Data from point%7i to%7i, with slow speed:"%(nDataFitStart,nDataFitEnd))

        if self.bitRateKnown == True:
            d=process_data(dataList,nDataFitStart,nDataFitEnd,lastFrequencyShift,lastBitRate,lastGoodFitTime,goFast,crcList)
        else:
            goFast=False
            lastBitRate=1200
            d=process_data(dataList,nDataFitStart,nDataFitEnd,lastFrequencyShift,lastBitRate,lastGoodFitTime,goFast,crcList)
            if d[3][3]==True:
                self.bitRateKnown = True
            else:
                lastBitRate=9600
                d=process_data(dataList,nDataFitStart,nDataFitEnd,lastFrequencyShift,lastBitRate,lastGoodFitTime,goFast,crcList)
                if d[3][3]==True:
                    self.bitRateKnown = True
                else:
                    lastBitRate=2400
                    d=process_data(dataList,nDataFitStart,nDataFitEnd,lastFrequencyShift,lastBitRate,lastGoodFitTime,goFast,crcList)
                    if d[3][3]==True:
                        self.bitRateKnown = True
                    else:
                        lastBitRate=4800
                        d=process_data(dataList,nDataFitStart,nDataFitEnd,lastFrequencyShift,lastBitRate,lastGoodFitTime,goFast,crcList)
                        if d[3][3]==True:
                            self.bitRateKnown = True

        self.paramList=append(self.paramList,array(d[3])[None,:],axis=0)
        self.paramList=self.paramList[self.paramList[-1,0]-self.paramList[:,0]<600]
        self.startTime,self.lastFrequencyShift,self.lastBitRate,self.oneGoodFrame,self.lastGoodFitTime=d[3]
        self.lastFreqChange=changeFreq(self.lastFrequencyShift,0.7e4,self.lastFreqChange,self.oneGoodFrame)
        self.d=d
        self.dataReady=True


def mainFunction(socket,decode):
    print("Starting Main Thread")

    sizeTmpBuffer=60000
    nDataFitEnd=0
    nDataFitStart=0
    dataList=[]

    while True:
        str1=socket.recv()
        if str1 == b'IQ':
            str2=socket.recv()
            dataPoints=unpack("i",str2)[0]
            proccess_data=False
            if (dataPoints==0)&(nDataFitEnd>2048):

                if nDataFitEnd-nDataFitStart < sizeTmpBuffer:
                    nDataFitStart=max([0,nDataFitEnd-sizeTmpBuffer])

                proccess_data=True
                endOfTransmission=True

            if (dataPoints!=0):
                floatPoints=dataPoints*2
                strData=socket.recv()
                dataValues=array(unpack("%if"%floatPoints,strData))
                dataList.append(dataValues[::2]+1j*dataValues[1::2])
                nDataFitEnd+=dataPoints
                if nDataFitEnd-nDataFitStart > sizeTmpBuffer:
                    proccess_data=True
                    endOfTransmission=False


            if proccess_data==True:

                while nDataFitStart > 300000:
                    npop=len(dataList.pop(0))
                    nDataFitStart-=npop
                    nDataFitEnd-=npop

                decode.dataList=dataList
                decode.nDataFitEnd=nDataFitEnd
                decode.nDataFitStart=nDataFitStart
                decode.run()

                if endOfTransmission==True:

                    decode.crcList=[]
                    dataList=[]
                    nDataFitStart=0
                    nDataFitEnd=0
                else:
                    delta=nDataFitEnd-nDataFitStart
                    nDataFitStart+=int(delta*0.8)


def printzmq(stringOutput):

    print(stringOutput)
    str1="MSG"
    socket2.send_string(str1,flags=zmq.SNDMORE)
    socket2.send_string(stringOutput)

def datazmq(data):

    str1="PACKET"
    socket2.send_string(str1,flags=zmq.SNDMORE)
    socket2.send(data)


def changeFreqzmq(relAbs,Hertz):

    str1="FREQ"
    socket2.send_string(str1,flags=zmq.SNDMORE)
    socket2.send_string(relAbs,flags=zmq.SNDMORE)
    socket2.send_string(Hertz)



msg="Decode"
print("\x1B]0;%s\x07" % msg)

context = zmq.Context()

# starting publisher for data packets
print("Starting ZMQ publisher…")
socket2 = context.socket(zmq.PUB)
socket2.bind("tcp://*:5564")
printzmq("Started ZMQ publisher.")


#  Socket to talk to Sylvain's C code
printzmq("Connecting to PicTalk")
socket = context.socket(zmq.SUB)
socket.connect("tcp://localhost:5563")

socket.setsockopt_string(zmq.SUBSCRIBE,"IQ")

printzmq("Connected to PicTalk.")

# configuring decode function
decode=Decode()

# configuring main thread
mainThread = Thread(target = mainFunction, args=(socket,decode))

# starting main thread
mainThread.start()


#resetThread = Thread(target = resetFreq, args=(socket2,decode))
#resetThread.start()
resetFreq(socket2,decode)

# check with dummy packet
#send_init_packet()
