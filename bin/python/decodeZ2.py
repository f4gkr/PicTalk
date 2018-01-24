
display= False
saveOutput= False

import time
from scipy.signal import welch
from scipy.signal import fftconvolve
import socket
from numpy import *  
from struct import *
import zmq
import requests
from threading import Thread, RLock

if display:
    import matplotlib
    matplotlib.use('Qt5Agg')
    from matplotlib.pyplot import *  
    
if saveOutput:
    import pickle
    
            
# VALIDATION CRC
def validate_CRC(frameRawHex,crc):
    toCheckBits=(unpackbits(frameRawHex[:,None],axis=1)[:,::-1].ravel()).astype('bool')
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
        
    crcBits=unpackbits(crc)[::-1]
    crcCheck=all(invert(SR)==crcBits) 
    
    return crcCheck,crcBits
              
def changeFreq(lastFrequencyShift,limit,lastFreqChange,detection):
    freq=435.250+lastFrequencyShift*1e-6    
    if detection:
        try:
            presentTime=time.time()
            if (abs(lastFrequencyShift) > limit)&(presentTime-lastFreqChange> 20) :
                r = requests.get('http://127.0.0.1:8001/status')
                freq=double(r.json()['rx_frequency'])+lastFrequencyShift*1e-6
                r = requests.get('http://127.0.0.1:8001/tune/'+str(freq))
                printzmq("New Frequency "+r.json()['frequency']+" MHz")
                lastFreqChange = presentTime
        except:
            printzmq("Error in Request to http")
        
    return lastFreqChange
                
    
def process_data(dataList,nDataFitStart,nDataFitEnd,lastFrequencyShift,lastBitRate,lastGoodFitTime,goFast,crcList):
            startTime=time.time()
            data=hstack(dataList)
            nData=len(data)
#            print(nData)
            
            nDataFitZero=nDataFitStart
            nDataFit=nDataFitEnd-nDataFitZero
            dataFit=data[nDataFitZero:nDataFitZero+nDataFit]
            if startTime-lastGoodFitTime>1:
                goFast=False
            
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
            gc=array([[interp(xFit+s,xp*f,real(dataFineCorrected))+1j*interp(xFit+s,xp*f,imag(dataFineCorrected)) for s in shiftBiteRate1] for f in freqBiteRate1])
            gc+=abs(real(gc))-real(gc)
            energy1=abs(gc.sum(axis=2))
            indexes=unravel_index(energy1.argmax(), energy1.shape)
            
#            print("..%i..with delay = %i ms"%(4,int((time.time()-startTime)*1000)))
            
            shiftBiteRate2=linspace(-0.1,0.1,10)*Nsample+shiftBiteRate1[indexes[1]]
            freqBiteRate2=linspace(-1.5e-4,1.5e-4,37)+freqBiteRate1[indexes[0]]
            gc=array([[interp(xFit+s,xp*f,real(dataFineCorrected))+1j*interp(xFit+s,xp*f,imag(dataFineCorrected)) for s in shiftBiteRate2] for f in freqBiteRate2])
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
                        crcCheck,crcBits=validate_CRC(frameRawHex[:-2],crc)
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
                    
            
#            print("..%i..with delay = %i ms"%(9,int((time.time()-startTime)*1000)))
            
            if saveOutput:
                if sum(frameChecking)<0.49*len(frameChecking):
                    if sum(frameChecking)>0.5:
                        if nData>120000:
                            a=(dataList,nDataFitStart,nDataFitEnd,lastFrequencyShift,lastBitRate,lastGoodFitTime,goFast,crcList)
                            d=(dataFinal,crcList,(phase1,phase2),(startTime,lastFrequencyShift,lastBitRate,oneGoodFrame),(startFrame,endFrame,frameChecking,frameCategory))
#                            with open("/home/picsat/data_decode/tmRawProc"+str(int(time.time()*100))+".pkl", 'wb') as f:
                            with open("/Users/slacour/PICSAT/ZeroMq/tmRawProc"+str(int(time.time()*100))+".pkl", 'wb') as f:
                                pickle.dump((a,d), f)
            
            return (dataFinal,crcList,(phase1,phase2),(startTime,lastFrequencyShift,lastBitRate,oneGoodFrame),(startFrame,endFrame,frameChecking,frameCategory))
        
        
        
class FigureDecode():
    def __init__(self):
        self.fig=figure("PicSat Decode",figsize=(6,4))
        self.fig.clf()
        self.ax1=self.fig.add_subplot(411)
        self.ax2=self.fig.add_subplot(412)
        self.ax3=self.fig.add_subplot(413)
        self.ax4=self.fig.add_subplot(414)
        self.plotNumber=0
        
    def figure_update(self, fig, ax1, ax2, ax3, ax4, decode):
    
        if decode.dataReady:
            d=decode.d
            paramList=decode.paramList.copy()
            paramList[:,0]-=paramList[0,0]
            good=paramList[:,-1]>0.5
            
            x=zeros(4*len(d[-1][0]))
            y=zeros(4*len(d[-1][0]))
            x[::4]=d[-1][0]
            x[1::4]=d[-1][0]
            x[2::4]=d[-1][1]
            x[3::4]=d[-1][1]
            y[::4]=logical_not(d[-1][2])
            y[1::4]=array(d[-1][2])>0.5
            y[2::4]=array(d[-1][2])>0.5
            y[3::4]=logical_not(d[-1][2])
            a=(d[-1][0]+d[-1][1])/2
            
            if (len(d[0])>0):
#                try:
#                    print("Plot Error")
#                except:
                        
                    ax1.clear()
                    ax2.clear()
                    ax3.clear()
                    ax4.clear()
                    
                    if len(d[-1][-1])>0:
                        ax1.set_title("Category "+str(d[-1][-1]))
                    else:
                        ax1.set_title("No Good Frame")
                    
                    ax1.plot(d[0])
                    ax1.plot(d[0],'.')
                    ax1.plot(x,2*(y-0.5)*max(abs(d[0])),'r')
                    if len(array(d[-1][2]))>0.5:
                        ax1.plot(a[array(d[-1][2])>0.5],a[array(d[-1][2])>0.5]*0,'og')
                    if len(array(d[-1][2]))>1.5:
                        ax1.plot(a[array(d[-1][2])>1.5],a[array(d[-1][2])>1.5]*0,'ob')
                    if len(logical_not(d[-1][2]))>0.5:
                        ax1.plot(a[logical_not(d[-1][2])],a[logical_not(d[-1][2])]*0,'or')
                    ax1.set_ylabel("Bits")
                    
                    ax2.plot(((d[2][0]-d[2][1]+pi)%2*pi)-pi)
                    ax2.plot((d[2][0]-d[2][0].min())*(1/(d[2][0].max()-d[2][0].min())*2*pi)-pi)
                    ax2.plot((d[2][1]-d[2][0].min())*(1/(d[2][0].max()-d[2][0].min())*2*pi)-pi)
                    ax2.set_ylabel("Phase (rad)")
                    
                    ax3.plot(paramList[good,0],paramList[good,1],'o-')
                    ax3.plot(paramList[logical_not(good),0],paramList[logical_not(good),1],'or')
                    ax3.set_ylim([-2e4,2e4])
                    ax3.set_xlim([0,600])
                    ax3.set_ylabel("Doppler (Hz)")
                    
                    if len(logical_not(good))>0.5:
                        ax4.plot(paramList[logical_not(good),0],paramList[logical_not(good),2],'or')
                    if len(good)>0.5:
                        ax4.plot(paramList[good,0],paramList[good,2],'o-')
                        bitRateMean=paramList[good,2].mean()
                        ax4.set_ylim([bitRateMean*0.995,bitRateMean*1.005])
                    ax4.set_xlim([0,600])
                    ax4.set_xlabel("time (seconds)")
                    ax4.set_ylabel("Bitrate")
                        
                    decode.dataReady= False
                    fig.canvas.draw()   
                    
                    
class Decode(Thread):
    def __init__(self):
        self.dataReady=False
        self.bitRateKnown=False
        self.lastFrequencyShift=0
        self.lastGoodFitTime=0
        self.lastFreqChange=0
        self.lastBitRate=9600
        self.bitRateknown=False
        self.goFast=False
        self.dataList=[]
        self.crcList=[]
        self.paramList=array([[0,0,0,0]])
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
            print("Proccessing Data from point %i to %i, with fast speed:"%(nDataFitStart,nDataFitEnd))
        else:
            print("Proccessing Data from point %i to %i, with slow speed:"%(nDataFitStart,nDataFitEnd))
        
        if self.bitRateKnown == True:
            d=process_data(dataList,nDataFitStart,nDataFitEnd,lastFrequencyShift,lastBitRate,lastGoodFitTime,goFast,crcList)
        else:
            goFast=False
            lastBitRate=9600
            d=process_data(dataList,nDataFitStart,nDataFitEnd,lastFrequencyShift,lastBitRate,lastGoodFitTime,goFast,crcList)
            if d[3][-1]==True:
                self.bitRateKnown = True
            else:
                lastBitRate=1200
                d=process_data(dataList,nDataFitStart,nDataFitEnd,lastFrequencyShift,lastBitRate,lastGoodFitTime,goFast,crcList)
                if d[3][-1]==True:
                    self.bitRateKnown = True
        
        self.paramList=append(self.paramList,array(d[3])[None,:],axis=0)
        self.paramList=self.paramList[self.paramList[-1,0]-self.paramList[:,0]<600]
        self.lastGoodFitTime,self.lastFrequencyShift,self.lastBitRate,self.oneGoodFrame=d[3]
        self.lastFreqChange=changeFreq(self.lastFrequencyShift,0.5e4,self.lastFreqChange,self.oneGoodFrame)
        self.d=d
        self.dataReady=True 
        
        
def mainFunction(socket,decode):
    print("Starting Main Thread")
    
    sizeTmpBuffer=50000
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
                    
                    nDataFitStart=nDataFitEnd


def printzmq(stringOutput):
    print(stringOutput)
    
    str1="MSG"
    socket2.send_string(str1,flags=zmq.SNDMORE)
    socket2.send_string(stringOutput)
    
def datazmq(data):
    
    str1="PACKET"
    socket2.send_string(str1,flags=zmq.SNDMORE)
    socket2.send(data)
           
context = zmq.Context()

# starting publisher for data packets
print("Starting ZMQ publisher…")
socket2 = context.socket(zmq.PUB)
socket2.bind("tcp://*:5564")
printzmq("Started ZMQ publisher.")
    

#  Socket to talk to server
printzmq("Connecting to PicTalk")
socket = context.socket(zmq.SUB)
socket.connect("tcp://localhost:5563")

socket.setsockopt_string(zmq.SUBSCRIBE,"IQ")

printzmq("Connected to PicTalk.")
    
# configuring decode function
decode=Decode()    

# configuring plots
if display:
    f1=FigureDecode()
    timer = f1.fig.canvas.new_timer(interval=300)
    timer.add_callback(f1.figure_update,f1.fig,f1.ax1,f1.ax2,f1.ax3,f1.ax4,decode)
    timer.start()

# configuring main thread
mainThread = Thread(target = mainFunction, args=(socket,decode))
mainThread.start()


# starting main thread
if __name__ == '__main__':
    mainFunction(socket,decode)
    
    
# plotting data
if __name__ == '__main__':
    if display:
        show()
