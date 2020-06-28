import paho.mqtt.client as mqtt
import time
import json 
import csv 
import ast
import os
import pandas as pd
#########################################################


#Received message callback
def on_message(client, userdata, message):
    f = open("data.json", "w")

    txt_file = open('data.txt', 'a') 
    txt_file.write(str(message.payload.decode("utf-8"))+"\n")
    txt_file.close()

    f.write(str(message.payload.decode("utf-8"))+"\n")
    print(str(message.payload.decode("utf-8")))
    f.close()

    data_store_csv(message.payload)

    with open('data.json') as json_file: 
        data = json.load(json_file)


# Function to store received data in a csv file
def data_store_csv(data1):

    # Define result array
    result = {"id":0,"f0:ec:af:cf:6c:e1": -200, "c9:a6:4d:9b:c0:8c": -200, "c2:b6:6e:70:fa:f7":-200, "d9:5f:f5:4f:10:89": -200, "c4:52:32:5c:31:e7": -200, "e9:3c:4a:34:13:fb": -200, "ed:61:e4:e8:22:30":-200, "ea:01:26:75:a4:c3": -200,"d0:4e:10:2e:cb:84":-200,"e4:e0:0a:ae:fd:e2": -200}



    # Define csv file path
    filename = "ble_data.csv"

    try:
        data = ast.literal_eval(data1.decode("utf-8"))
        print(data)

        for key,value in data.items():
            if key in result and result[key] == -200:
                result[key] = float(value)
        
        result["id"] = int(data["id"])

        # Create one row of csv file
        df = pd.Series(result).to_frame().T

        print(df)

        # Write to csv file
        df.to_csv(filename,index=False,mode='a',header=(not os.path.exists(filename)))
        
    except Exception as e:
        print(e)

client =mqtt.Client("Nalith")
client.connect("mqtt.eclipse.org")
client.on_message=on_message
csv_file = open('ble_data.csv', 'w') 
csv_file.write("")
csv_file.close()
txt_file = open('data.txt', 'w') 
txt_file.write("")
txt_file.close()


client.subscribe("EN3250/ESP32")
client.loop_forever()
# time.sleep(100000) 
# client.loop_stop() 
