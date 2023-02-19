import picamera
import cv2
import os
import RPi.GPIO as GPIO
# install pyserial: sudo apt-get install python-serial
import serial
import threading
import socket
import time
import numpy


# BEGIN INIT

PIN_IN_SCHEDULE_EXE = 2
PIN_IN_FIND_CAT_TIMEOUT = 3
PIN_IN_SCHEDULE_END = 8
PIN_OUT_FOUND_CAT = 23

# socket
tcpDta = 0

# GPIO 23, 24, 25 is OUTPUT
# GPIO 02, 03, 08, 09 is INPUT(pull-up)
#http://lhdangerous.godohosting.com/wiki/index.php/Raspberry_pi_%EC%97%90%EC%84%9C_python%EC%9C%BC%EB%A1%9C_GPIO_%EC%82%AC%EC%9A%A9%ED%95%98%EA%B8%B0
GPIO.setmode(GPIO.BCM)
GPIO.setup(23, GPIO.OUT)
GPIO.setup(24, GPIO.OUT)
GPIO.setup(25, GPIO.OUT)
GPIO.setup(PIN_IN_SCHEDULE_EXE, GPIO.IN, pull_up_down = GPIO.PUD_UP)
GPIO.setup(PIN_IN_FIND_CAT_TIMEOUT, GPIO.IN, pull_up_down = GPIO.PUD_UP)
GPIO.setup(PIN_IN_SCHEDULE_END, GPIO.IN, pull_up_down = GPIO.PUD_UP) #
GPIO.setup(9, GPIO.IN, pull_up_down = GPIO.PUD_UP) #
GPIO.output(PIN_OUT_FOUND_CAT, GPIO.LOW) # found cat
GPIO.output(24, GPIO.LOW) # 
GPIO.output(25, GPIO.LOW) #

# END INIT

# BEGIN FUNDAMENTAL FUNC
def chkCat(): # capture cam, check if cat exists
#https://pythonprogramming.net/raspberry-pi-camera-opencv-face-detection-tutorial/
#https://m.blog.naver.com/PostView.naver?isHttpsRedirect=true&blogId=3demp&logNo=221441776368
    cap = cv2.VideoCapture("http://127.0.0.1:8080/?action=stream")
    ret, frame = cap.read()
    cascade = '/usr/local/share/OpenCV/haarcascades/haarcascade_frontalcatface.xml'
    face_cascade = cv2.CascadeClassifier(cascade)
    if (ret):
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)    
        faces = face_cascade.detectMultiScale(gray, 1.1, 5)
        return len(faces)
    else return 0

def playAudio(number):
    # do not program this before fundamentals

# END FUNDAMENTAL FUNC

# BEGIN THREADED FUNC

def thr_conn():
    # start serial
    ser = serial.Serial('/dev/ttyAMA0', 9600, timeout=1)
    ser.open()

    #https://m.blog.naver.com/PostView.naver?isHttpsRedirect=true&blogId=zeta0807&logNo=222144886241
    HOST = '192.168.1.9'
    PORT = '9903'
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDP, 1)
    s.bind((ip, port))
    s.listen()
    while 1:
        clientSock, addr = s.accept()
        while 1:
            global tcpDta
            tcpDta = clientSock.recv(8)
            if not tcpDta:
                break
            else:
                ser.write(tcpDta)

# END THREADED FUNC

# BEGIN APP

def find():
    while 1:
        found = chkCat()
        if found == 1:
            GPIO.output(PIN_OUT_FOUND_CAT, GPIO.HIGH)
            time.sleep(1)
            GPIO.output(PIN_OUT_FOUND_CAT, GPIO.LOW)
            break
        else:
            if GPIO.input(PIN_IN_FIND_CAT_TIMEOUT):
                break

def main():
    isRun = False
    while 1:
        if GPIO.input(PIN_IN_SCHEDULE_EXE): # schedule start
            isRun = True
            find()
        elif GPIO.input(PIN_IN_SCHEDULE_END)
            isRun = False

# END APP

# EXECUTE APP
while 1:
    main()
