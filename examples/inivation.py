import event_camera_drivers as ec
import time

cam = ec.InivationCamera()

print(cam.resolution())

total_events = 0
current_second_events = 0
start_time = time.time()
last_second = int(start_time)
elapsed_time = 0

for events in cam:
    events_count = len(events)
    total_events += events_count
    current_second_events += events_count
    
    current_time = time.time()
    current_second = int(current_time)
    elapsed_time = current_time - start_time
    
    if current_second > last_second:
        print(f"{last_second - int(start_time)}s: {current_second_events} events")
        current_second_events = 0
        last_second = current_second
    
    print(f"Current events this second: {current_second_events}", end='\r')

print(f"\nTotal events: {total_events}, Average events/sec: {total_events / elapsed_time:.2f}")

