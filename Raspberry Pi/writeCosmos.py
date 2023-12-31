from datetime import datetime, timedelta
from pymongo import MongoClient
from azure.cosmos import CosmosClient
import json

mongo_client = MongoClient('mongodb://gHospitalAdmin:gHospitalAdmin@192.168.10.99:27017/')
mongo_collection = mongo_client.get_database("greenHospital").get_collection("sensorReadings")

cosmos_uri = ""   #Add cosmos URI
cosmos_key = ""   #Add cosmos Key
cosmos_client = CosmosClient(cosmos_uri, cosmos_key)
cosmos_db = cosmos_client.get_database_client("hospital-Birmingham")
cosmos_container = cosmos_db.get_container_client("sensorReadings")

def serialize(obj):
    return obj.isoformat()

def calculate_and_store_aggregated_data():
    # Calculate aggregated data from the last 30 minutes
    time_period = datetime.utcnow() - timedelta(minutes=30)
    data = mongo_collection.find({"timestamp": {"$gte": time_period}})

    # Calculate average, min, max for each sensor type
    aggregated_data = {}
    for entry in data:
        timestamp = entry["timestamp"]
        parameters = entry["data"]
        
        for parameter, value in parameters.items():

            if parameter not in aggregated_data:
                aggregated_data[parameter] = {
                    "total": 0,
                    "count": 0,
                    "min": float('inf'),
                    "max": float('-inf')
                }

            aggregated_data[parameter]["total"] += value
            aggregated_data[parameter]["count"] += 1
            aggregated_data[parameter]["min"] = min(aggregated_data[parameter]["min"], value)
            aggregated_data[parameter]["max"] = max(aggregated_data[parameter]["max"], value)

    # Calculate average for each sensor type
    for parameter, stats in aggregated_data.items():
        if stats["count"] > 0:
            average = stats["total"] / stats["count"]
            aggregated_data[parameter]["average"] = average
        else:
            aggregated_data[parameter]["average"] = None

    # Store aggregated data in Cosmos DB
    document_id = str(datetime.utcnow())
    cosmos_container.upsert_item({
        "id": document_id,
        "timestamp": datetime.utcnow().isoformat(),
        "aggregated_data": json.loads(json.dumps(aggregated_data, default=serialize))
    })

if __name__ == "__main__":
    calculate_and_store_aggregated_data()
