import cv2
import mediapipe as mp
import time
import threading
import serial
from picamera2 import Picamera2

mp_drawing = mp.solutions.drawing_utils
mp_hands = mp.solutions.hands

picam2 = Picamera2()
picam2.configure(picam2.create_preview_configuration(main={"format":"RGB888","size":(640,480)}))
picam2.start()
width = 640
height = 480

#ser = serial.Serial('/dev/ttyACM0',115200)
#ser.reset_input_buffer()

cv2.startWindowThread()

# Initialize hand tracking with increased min_detection_confidence
hands = mp_hands.Hands(
    static_image_mode=False,
    max_num_hands=1,  # Detect only one hand
    min_detection_confidence=0.4,  # Increase the confidence threshold
    min_tracking_confidence=0.5,
)

startMarker = '<'
endMarker = '>'
dataStarted = False
dataBuf = ""
messageComplete = False

#========================
#========================
    # the functions

def setupSerial(baudRate, serialPortName):
    
    global  serialPort
    
    serialPort = serial.Serial(port= serialPortName, baudrate = baudRate, timeout=0, rtscts=True)

    print("Serial port " + serialPortName + " opened  Baudrate " + str(baudRate))

    waitForArduino()

#========================

def sendToArduino(stringToSend):
    
        # this adds the start- and end-markers before sending
    global startMarker, endMarker, serialPort
    
    stringWithMarkers = (startMarker)
    stringWithMarkers += stringToSend
    stringWithMarkers += (endMarker)

    serialPort.write(stringWithMarkers.encode('utf-8')) # encode needed for Python3


#==================

def recvLikeArduino():

    global startMarker, endMarker, serialPort, dataStarted, dataBuf, messageComplete

    if serialPort.inWaiting() > 0 and messageComplete == False:
        x = serialPort.read().decode("utf-8") # decode needed for Python3
        
        if dataStarted == True:
            if x != endMarker:
                dataBuf = dataBuf + x
            else:
                dataStarted = False
                messageComplete = True
        elif x == startMarker:
            dataBuf = ''
            dataStarted = True
    
    if (messageComplete == True):
        messageComplete = False
        return dataBuf
    else:
        return "XXX" 

#==================

def waitForArduino():

    # wait until the Arduino sends 'Arduino is ready' - allows time for Arduino reset
    # it also ensures that any bytes left over from a previous message are discarded
    
    print("Waiting for Arduino to reset")
     
    msg = ""
    while msg.find("Arduino is ready") == -1:
        msg = recvLikeArduino()
        if not (msg == 'XXX'): 
            print(msg)


def track_hands():
#     cap = cv2.VideoCapture(0)
    
    history_length = 5  # Length of position history
    threshold_x = 50  # Decreased threshold for horizontal movement
    threshold_y = 5  # Decreased threshold for vertical movement
    hand_x_position_history = [0] * history_length
    hand_y_position_history = [0] * history_length
    


    current_direction = ""
    setupSerial(115200, "/dev/ttyUSB0")
    count = 0
    prevTime = time.time()

    while True:
        im = picam2.capture_array()
        results = hands.process(cv2.cvtColor(im, cv2.COLOR_BGR2RGB))
        
        #line = ser.readline()
        #print("Read " + line.decode('utf-8', errors='ignore').strip()+ "from arduino")

        if results.multi_hand_landmarks:
            hand_landmarks = results.multi_hand_landmarks[0]  # Use the first detected hand

            tip_x_position = hand_landmarks.landmark[mp_hands.HandLandmark.MIDDLE_FINGER_TIP].x * width
            mcp_x_position = hand_landmarks.landmark[mp_hands.HandLandmark.MIDDLE_FINGER_MCP].x * width
            tip_y_position = hand_landmarks.landmark[mp_hands.HandLandmark.MIDDLE_FINGER_TIP].y * height
            mcp_y_position = hand_landmarks.landmark[mp_hands.HandLandmark.MIDDLE_FINGER_MCP].y * height

            hand_x_position_history.append(tip_x_position - mcp_x_position)
            hand_y_position_history.append(tip_y_position - mcp_y_position)

            if len(hand_x_position_history) > history_length:
                hand_x_position_history.pop(0)
            if len(hand_y_position_history) > history_length:
                hand_y_position_history.pop(0)

            smoothed_x_position = sum(hand_x_position_history) / history_length
            smoothed_y_position = sum(hand_y_position_history) / history_length
            
#             arduinoReply = recvLikeArduino()
#             if (arduinoReply == 'Reset'):
#                 serialPort.flush()
#                 serialPort.reset_input_buffer()
#                 serialPort.reset_output_buffer()
                
#             if not (arduinoReply == 'XXX'):
#                 print("Time %s  Reply %s" %(time.time(), arduinoReply))
            
            if smoothed_x_position < -threshold_x:
                current_direction = "C"
              #  ser.write("A\n".encode('utf-8'))
            elif smoothed_x_position > threshold_x:
                current_direction = "A"
              #  ser.write("D\n".encode('utf-8'))
            elif smoothed_y_position < -threshold_y:
                current_direction = "B"
               # ser.write("W\n".encode('utf-8'))
            elif smoothed_y_position > threshold_y:
                current_direction = "D"
               # ser.write("S\n".encode('utf-8'))

            if time.time() - prevTime > 0.1:
                sendToArduino(current_direction)
                prevTime = time.time()
           
           
            # Clear the screen and display the current direction
            print("\033[H\033[J")
            if current_direction:
                print("Move " + current_direction)

                
#                 ser.write(str(current_direction+"\n").encode('utf-8'))
                

#             mp_drawing.draw_landmarks(frame, hand_landmarks, mp_hands.HAND_CONNECTIONS)

        cv2.imshow('Frame', im)
        if cv2.waitKey(1) & 0xFF == 27:
            break


    cv2.destroyAllWindows()

if __name__ == "__main__":
    tracking_thread = threading.Thread(target=track_hands)
    tracking_thread.start()

    tracking_thread.join()
