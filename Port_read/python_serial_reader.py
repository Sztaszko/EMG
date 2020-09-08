from sys import exit
import serial
from optparse import OptionParser
import time
import msvcrt

parser = OptionParser()
parser.add_option("-o", "--output-file", dest="filename",
                  help="write output to CSV, and log", metavar="FILE")
parser.add_option("-p", "--serial-port1", dest="port1",
                  help="device serial port")
parser.add_option("-e", "--serial-port2", dest="port2",
                  help="device serial port for emg")
parser.add_option("-n", "--num-samples", dest="num_samp",
                  help="number of samples to collect (for force)", default=0, type='int')

(options, args) = parser.parse_args()

filename = options.filename
filename_emg = filename + '_emg'
if options.filename == None:
    print('No output file name added')
#    filename = 'out'
    exit()

port1 = options.port1
if options.port1 == None:
    print('No serial port 1 added')
#    port = 'COM5'
    exit()

port2 = options.port2
if options.port2 == None:
    print('No serial port 2 added')
#    port = 'COM4'
    exit()

num_samp = options.num_samp

samp_time_s = (0.1)

ser = serial.Serial(port1, 9600, serial.EIGHTBITS, serial.PARITY_NONE, serial.STOPBITS_ONE, 1000)
#ser.open()
ser_emg = serial.Serial(port2, 9600, serial.EIGHTBITS, serial.PARITY_NONE, serial.STOPBITS_ONE, 1000)


log_file = open(filename+".log","a+")
out_file = open(filename+".csv","w+", buffering=1)
out_file_emg = open(filename_emg+".csv","w+", buffering=1)

log_file.write('python_serial_reader.py\n')
start_time = time.time()
log_file.write('start at: %s\n'% (time.asctime( time.localtime(start_time))))
print('start at: %s\n'% (time.asctime( time.localtime(start_time))))
log_file.write('force output filename: %s\n'% (filename))
print('force output filename: %s\n'% (filename))
log_file.write('emg output filename: %s\n'% (filename_emg))
print('emg output filename: %s\n'% (filename_emg))
log_file.write('serial port1: %s\n'% (port1))
print('serial port1: %s\n'% (port1))
log_file.write('serial port2: %s\n'% (port2))
print('serial port2: %s\n'% (port2))
log_file.write('number of force samples to collect: %d\n'% (num_samp))
print('number of force samples to collect: %d\n'% (num_samp))
log_file.write('number of emg samples: %d\n'% (num_samp*100))
log_file.write('time of acquisition (s): ~%d\n'% (num_samp*0.1))
log_file.write('unit of force: kg\nunit of emg (values 0-255): - ')


global_point = 0

#out_file.write('local_time_from_app_start,arduino_time_ms,arduino_measurement_period,value\n');


def readValue(serialPort):
    return(ord(serialPort.read(1)))

'''
def read_port(serialPort, samples_emg):
    cur_time = time.time() + 0.0009 
    if num_samp>0:
        num_samp_emg=97.6*num_samp
        while(num_samp>0):
            newValue=readValue(serialPort)
            out_file_emg.write(newValue)
            num_samp_emg-=1
            sleep_time = cur_time - time.time()  
            time.sleep(sleep_time)
    else:
        while True:
            newValue=readValue(serialPort)
            out_file_emg.write(newValue)
            num_samp_emg-=1
            sleep_time = cur_time - time.time()  
            time.sleep(sleep_time)
            if msvcrt.kbhit():
                s_char = msvcrt.getch().decode('utf-8')
                if s_char == 'q':
                    break
'''

def run_once(g_m):    
    cur_time = time.time() + samp_time_s
    str_val = ser.read_until().decode('ascii')
    str_val = str_val[:-2]
    f_val= abs(float(str_val))
    #time_mesure = time.time()-start_time
    out_file.write('{:.2f},\n'.format(f_val))
    print(f_val)
    for i in range(100):
        newValue=readValue(ser_emg)
        out_file_emg.write(str(newValue)+',\n')
    sleep_time = cur_time - time.time() 
    if sleep_time > 0.0:
        time.sleep(sleep_time)
        

ser_emg.flush()
ser_emg.isOpen()
reading_start_time=time.time()
if num_samp > 0:
    print("start odczytu...\n")
    while num_samp > 0:
        run_once(global_point)
        num_samp -= 1
        
else:
    print("start odczytu...\n")
    while True:
        run_once(global_point)
        if msvcrt.kbhit():
            s_char = msvcrt.getch().decode('utf-8')
            if s_char == 'q':
                break

print("koniec odczytu")
print("czas pomiaru: ", time.time()-reading_start_time)
log_file.write('\nend of run\n\n')
log_file.close()
out_file.close()
ser.close()
