import serial
import csv
import time

def readValue(serialPort):
    return(ord(serialPort.read(1)))

def plotValue(value):
    #Wyświetla wartość na skalowanych i przewijanych słupkach
    leadingSpaces = " " * int((value*(SCREEN_WIDTH-3) / 255))
    print("%s%3i" % (leadingSpaces, value))

"""
class Timer(threading.Thread):
    def __init__(self,time,target):
        threading.Thread.__init__(self)
        self.target=target
        self.time=time
        self.flag=True
        self.start()
        
    def run(self):
        while self.target():
            sleep(self.time)

"""



if __name__ == "__main__":
    
    PORT = 'COM4'
    BAUDRATE =  9600
    TIMEOUT = None
    SCREEN_WIDTH = 80
    to_save_values = []
    ELAPSE_TIME=5
    
    
    ser = serial.Serial(PORT, 
                        BAUDRATE, 
                        timeout=TIMEOUT, 
                        stopbits=serial.STOPBITS_ONE, 
                        bytesize=serial.EIGHTBITS)
    ser.isOpen()
    ser.flush()
    command = "test"
    ser.write(command.encode())
    
    wait_s = 1
    received = None
    
    #while received == ser.read(wait_s):
    newValue=readValue(ser)
    
    start=time.time()
    while(1):
        if time.time()-start<ELAPSE_TIME:
            newValue = readValue(ser)
            plotValue(newValue)
            to_save_values.append(newValue)
        else:
            break
    ser.close()


    with open('pomiar_testowy.csv', mode='w') as measurement_file:
        measure_writer=csv.writer(measurement_file, delimiter=',')
        measure_writer.writerow(to_save_values)
    



    



  

