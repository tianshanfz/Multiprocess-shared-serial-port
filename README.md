Multiprocess-shared-serial-port
===============================


    Os such as win or linux allows no more than one process to use the serial port simultaneously. Sometimes , we want to write commands to  lower machine while another process is reading data continuously.This is a solvement.
    
    This program is only for communications in request-respond mode.It adds a head before each data sent and recieved between  upper machine and lower machine,indicating which process sends(recieves) it. And a process managing the data queue is running background.
    
--written by hyb for a robot.
