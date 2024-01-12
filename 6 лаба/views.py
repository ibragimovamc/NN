from django.shortcuts import render
from django.http import HttpResponse
import sqlite3
import json
from datetime import datetime


def all_logs(request):
    db_path = '/Users/ivan/CLionProjects/fourth_lab_operations_system/cmake-build-debug/log_database.dblite'
    con = sqlite3.connect(db_path)
    cursor = con.cursor()
    cursor.execute("SELECT * FROM log")
    records = cursor.fetchall()
    record_dict= {}
    for record in records:
        record_dict[record[2]] = record[1]
    json_records = json.dumps(record_dict)
    return HttpResponse(json_records)


def get_filter_records(request):
    dict_data = json.loads(request.body.decode('utf-8').replace("'", '"'))

    db_path = '/Users/ivan/CLionProjects/fourth_lab_operations_system/cmake-build-debug/log_database.dblite'
    con = sqlite3.connect(db_path)
    cursor = con.cursor()
    cursor.execute("SELECT * FROM log")

    records = cursor.fetchall()
    record_dict = {}
    date_format_back = '%Y-%m-%d %H:%M:%S'

    try:
        timestamp_start = datetime.strptime(dict_data['start'], date_format_back).timestamp()
        timestamp_end = datetime.strptime(dict_data['end'], date_format_back).timestamp()
    except:
        dict_data['start'] = dict_data['start'].replace('T', ' ') + ":00"
        dict_data['end'] = dict_data['end'].replace('T', " ") + ":00"

    for record in records:
        date_obj_record = datetime.strptime(record[2], date_format_back).timestamp()
        if float(timestamp_start) <= date_obj_record <= float(timestamp_end):
            record_dict[record[2]] = record[1]

    json_recs = json.dumps(record_dict)
    return HttpResponse(json_recs)


def get_current_temperature(request):
    db_path = '/Users/ivan/CLionProjects/fourth_lab_operations_system/cmake-build-debug/log_database.dblite'
    con = sqlite3.connect(db_path)
    cursor = con.cursor()
    #current_time = "2023-12-30 18:39:52"
    current_time = str(datetime.now())
    cursor.execute("SELECT * FROM log WHERE date =?", (current_time, ))
    rec = cursor.fetchall()
    json_rec = json.dumps({'temperature': rec[0][1]})
    return HttpResponse(json_rec)