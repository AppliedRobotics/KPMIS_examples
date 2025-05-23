import serial #Подключаем библиотеку для работы с последовательным портом
from subprocess import call #Подключаем функцию для вызова сервисов

port = "/dev/ttyS2"  #Объявляем порт, через который камера получает данные
baudrate = 115200   #Объявляем скорость обмена данными 
flag = -1   #Создаём флаг для того, чтобы при многократном отправлении одной и той же команды в порт, программа исполняла команду только один раз
try:
    ser = serial.Serial(port, baudrate=baudrate)    #Создаём дескриптор порта
    print("Serial connection established.") #В случае успеха, выводим в терминал сообщение
    
    while True:
        line = ser.readline().decode().strip()  #Считываем строку байт из буффера и переводим в string

        if (line == "mcx" and flag != 0):   #Если мы передали сообщение mcx, camera-server выключается, а mcx-tracking-cam включается
            call(["systemctl", "stop", "camera-server"])    
            call(["systemctl", "start", "mcx-tracking-cam"])    
            print("mcx-tracking-cam ON")    
            flag = 0    
        elif (line == "server" and flag != 1):  #Если мы передали сообщение server, camera-server включается, а mcx-tracking-cam выключается
            call(["systemctl", "stop", "mcx-tracking-cam"]) 
            call(["systemctl", "start", "camera-server"])   
            print("camera-server ON")   
            flag = 1    
except serial.SerialException as se:    #Срабатывает при ошибке 
    print("Serial port error:", str(se))

except KeyboardInterrupt:
    pass

finally:    #При остановке скрипта закрывается порт, выключаются сервисы
    if ser.is_open: #
        ser.close() #
        print("camera-server and mcx-tracking-cam OFF")
        call(["systemctl", "stop", "mcx-tracking-cam"]) 
        call(["systemctl", "stop", "camera-server"])    
        print("Serial connection closed.")
