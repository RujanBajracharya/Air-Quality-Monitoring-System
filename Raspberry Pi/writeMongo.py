import paho.mqtt.client as mqtt
from datetime import datetime
import time
from datetime import datetime
from pymongo import MongoClient
from bson import Binary, ObjectId

mongoClient = MongoClient('mongodb://gHospitalAdmin:gHospitalAdmin@192.168.10.99:27017/')
collection = mongoClient.get_database("greenHospital").get_collection("sensorReadings")

def on_connect(client, userdata, flags, rc):
    print("Connected\n")
    client.subscribe("esp32/rujanHospital/#")
    
    
def on_message(client, userdata, msg):
    print("New message on topic "+msg.topic+": " + str(msg.payload)[1::])
    object_id = ObjectId()

    data = {
        "_id": object_id,
        "timestamp": datetime.utcnow(),
        "data": {
            msg.topic[20::]: float(msg.payload)
        }
                
    }

    collection.insert_one(data)
        
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect("test.mosquitto.org")
client.loop_start()