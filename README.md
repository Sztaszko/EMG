# EMG
Receiving EMG signal within the project of TechMed Science Club in AGH Cracow.


<b>python_serial_reader.py options: </b>

-o --output-file ;filename of force and log file. File containing EMG signal will have additional "_emg" in name. All data is saved in CSV.

-p --serial-port1 ;name of serial port for force. For example COM5

-e --serial-port2 ;name of serial port for emg signal.

-n --num-samples ;amount of force samples to collect. Force samples from dynamometer are received at 10 Hz rate. EMG signal is sampled at ~1kHz.


