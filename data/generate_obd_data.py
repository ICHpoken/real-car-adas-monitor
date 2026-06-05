import csv
import random

# Используем нормальное распределение с перекрытием классов
# Это делает задачу более реалистичной

def generate_record():
    # Случайно выбираем стиль вождения
    label = random.choice(['SLOW', 'NORMAL', 'AGGRESSIVE'])
    
    # Параметры для нормального распределения (среднее, разброс)
    if label == 'SLOW':
        speed = random.gauss(45, 20)      # Среднее 45, разброс 20
        rpm = random.gauss(1800, 600)     # Среднее 1800, разброс 600
        throttle = random.gauss(20, 12)   # Среднее 20, разброс 12
    elif label == 'NORMAL':
        speed = random.gauss(75, 18)      # Среднее 75, разброс 18
        rpm = random.gauss(3000, 600)     # Среднее 3000, разброс 600
        throttle = random.gauss(45, 18)   # Среднее 45, разброс 18
    else:  # AGGRESSIVE
        speed = random.gauss(110, 22)     # Среднее 110, разброс 22
        rpm = random.gauss(4500, 800)     # Среднее 4500, разброс 800
        throttle = random.gauss(80, 15)   # Среднее 80, разброс 15
    
    # Ограничиваем значения реалистичными диапазонами
    speed = max(0, min(200, speed))
    rpm = max(500, min(7000, rpm))
    throttle = max(0, min(100, throttle))
    
    # Остальные параметры (не зависят от стиля)
    coolant_temp = random.gauss(92, 8)
    coolant_temp = max(60, min(120, coolant_temp))
    
    fuel_level = random.uniform(10, 100)
    intake_air_temp = random.gauss(25, 8)
    intake_air_temp = max(0, min(50, intake_air_temp))
    
    return [
        round(speed, 1),
        round(rpm, 0),
        round(throttle, 1),
        round(coolant_temp, 1),
        round(fuel_level, 1),
        round(intake_air_temp, 1),
        label
    ]

# Генерируем 5000 строк
with open('data/obd_data.csv', 'w', newline='', encoding='utf-8') as f:
    writer = csv.writer(f)
    writer.writerow(['speed_kmh', 'engine_rpm', 'throttle_pos', 
                     'coolant_temp', 'fuel_level', 'intake_air_temp', 'label'])
    
    for _ in range(5000):
        record = generate_record()
        writer.writerow(record)

print("✅ Сгенерировано 5000 строк с реалистичным перекрытием классов")