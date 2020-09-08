import threading

import serial
import csv
import time

def readValue(serialPort):
    return(ord(serialPort.read(1)))

def plotValue(value):
    #Wyświetla wartość na skalowanych i przewijanych słupkach
    leadingSpaces = " " * int((value*(SCREEN_WIDTH-3) / 255))
    print("%s%3i" % (leadingSpaces, value))
    
def receive_force(serialPort, force_data_container, elapse_time):
    start=time.time()
    while(1):
        if time.time()-start<elapse_time:      
            newValue_force=readValue(serialPort)
            force_data_container.append(newValue_force)
        else:
            break
        
    

if __name__ == "__main__":
    
    PORT_EMG = 'COM4'
    PORT_FORCE = 'COM5'
    BAUDRATE =  9600
    TIMEOUT = None
    SCREEN_WIDTH = 80
    EMG_values = []
    FORCE_values = []
    ELAPSE_TIME=10
    
    
    ser_force = serial.Serial(PORT_FORCE, 
                        BAUDRATE, 
                        timeout=TIMEOUT, 
                        stopbits=serial.STOPBITS_ONE, 
                        bytesize=serial.EIGHTBITS)
    
    ser_emg= serial.Serial(PORT_EMG,
                        BAUDRATE,
                        timeout=TIMEOUT, 
                        stopbits=serial.STOPBITS_ONE, 
                        bytesize=serial.EIGHTBITS)
    ser_force.isOpen()
    ser_force.flush()
    
    ser_emg.isOpen()
    ser_emg.flush()
    #command = "test"
    #ser.write(command.encode())
    
    wait_s = 1
    received = None
    
    #while received == ser.read(wait_s):
    #newValue_force=readValue(ser_force)
    #newValue_emg=readValue(ser_emg)
    
    FORCE_thread= threading.Thread(target=receive_force, args=(ser_force,FORCE_values,ELAPSE_TIME))
    
    
    start=time.time()
    FORCE_thread.start()
    while(1):
        if time.time()-start<ELAPSE_TIME:      
            newValue_emg=readValue(ser_emg)
            EMG_values.append(newValue_emg)
            plotValue(newValue_emg)
        else:
            break
    ser_emg.close()
    ser_force.close()

    with open('szybki_zacisk_wolne_puszczanie_emg1.csv', mode='w') as measurement_file:
        measure_writer=csv.writer(measurement_file, delimiter=',')
        measure_writer.writerow(EMG_values)


    with open('szybki_zacisk_wolne_puszczanie_force1.csv', mode='w') as measurement_file:
        measure_writer=csv.writer(measurement_file, delimiter=',')
        measure_writer.writerow(FORCE_values)
    
    import matplotlib.pyplot as plt
    plt.figure(1)
    plt.plot(EMG_values)
    plt.title('EMG')
    plt.xlabel('Czas [~ms]')
    plt.ylabel('Wartosc sygnału [0-255]')
    plt.show()
    
    plt.figure(2)
    plt.plot(FORCE_values)
    plt.title('FORCE')
    plt.show()


    
