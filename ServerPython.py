
import socket
import sys
import requests

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

ip_address = "";
port_number = 0;

server_address = (ip_address, port_number)
sock.bind(server_address)
sock.listen(1)

while True:
    
    print('Oczekanie na połączenie')
    connection, client_address = sock.accept()
    try:
        while True:
            data = connection.recv(1024)
            if data:
                print('Otrzymano: ', data)

                device, token,type,operation,new_value = data.decode().split("____")

                print(device)
                print(token)
                print(type)
                print(operation)
                print(new_value)

                
                if (operation == "U"):
                    update_headers = {
                        'Content-Type': 'application/json',
                        'X-Authorization': 'Bearer eyJhbGciOiJIUzUxMiJ9.eyJzdWIiOiJ0b21hc3oua3Jva29zekBnbWFpbC5jb20iLCJ1c2VySWQiOiIxMTY0ZDU5MC01YTRjLTExZWYtYWFjNS1kMTRlMTgwYWZkZjUiLCJzY29wZXMiOlsiVEVOQU5UX0FETUlOIl0sInNlc3Npb25JZCI6IjQ4OTNjYjUzLTY1NTMtNGIwMC04OWEzLTZhODg4N2M1NjNhMSIsImV4cCI6MTcyNTU3OTM2OSwiaXNzIjoidGhpbmdzYm9hcmQuY2xvdWQiLCJpYXQiOjE3MjU1NTA1NjksImZpcnN0TmFtZSI6IlRvbWFzeiIsImxhc3ROYW1lIjoiS3Jva29zeiIsImVuYWJsZWQiOnRydWUsImlzUHVibGljIjpmYWxzZSwiaXNCaWxsaW5nU2VydmljZSI6ZmFsc2UsInByaXZhY3lQb2xpY3lBY2NlcHRlZCI6dHJ1ZSwidGVybXNPZlVzZUFjY2VwdGVkIjp0cnVlLCJ0ZW5hbnRJZCI6IjExMjZiYjIwLTVhNGMtMTFlZi1hYWM1LWQxNGUxODBhZmRmNSIsImN1c3RvbWVySWQiOiIxMzgxNDAwMC0xZGQyLTExYjItODA4MC04MDgwODA4MDgwODAifQ.o87LyzShhh5RML5KHjQgUni-bf1zi3LJdMHpRfYTYGnUthj3H4x2edCnm6zj46UHX-R0F4uogMiV8kCe6SJJ3g',

                    }

                    update_data = '{"'+ type +'": ' + new_value + '}'
                    #example {'temperature': 23}

                    response_update = requests.post('http://thingsboard.cloud/api/v1/' + token +'/telemetry', headers=update_headers, data=update_data)

                 #   response_max = requests.get('http://cloud.thingsboard.io/api/plugins/telemetry/DEVICE/' + device + '/values/timeseries', headers=headers_max_temp, params=params_max_tem)
                    print(response_update.content)
                    connection.sendall(response_update.content)
                    break
                if (operation == "R"):
                    headers = {
                        'Content-Type': 'application/json',
                        'X-Authorization': 'Bearer eyJhbGciOiJIUzUxMiJ9.eyJzdWIiOiJ0b21hc3oua3Jva29zekBnbWFpbC5jb20iLCJ1c2VySWQiOiIxMTY0ZDU5MC01YTRjLTExZWYtYWFjNS1kMTRlMTgwYWZkZjUiLCJzY29wZXMiOlsiVEVOQU5UX0FETUlOIl0sInNlc3Npb25JZCI6IjQ4OTNjYjUzLTY1NTMtNGIwMC04OWEzLTZhODg4N2M1NjNhMSIsImV4cCI6MTcyNTU3OTM2OSwiaXNzIjoidGhpbmdzYm9hcmQuY2xvdWQiLCJpYXQiOjE3MjU1NTA1NjksImZpcnN0TmFtZSI6IlRvbWFzeiIsImxhc3ROYW1lIjoiS3Jva29zeiIsImVuYWJsZWQiOnRydWUsImlzUHVibGljIjpmYWxzZSwiaXNCaWxsaW5nU2VydmljZSI6ZmFsc2UsInByaXZhY3lQb2xpY3lBY2NlcHRlZCI6dHJ1ZSwidGVybXNPZlVzZUFjY2VwdGVkIjp0cnVlLCJ0ZW5hbnRJZCI6IjExMjZiYjIwLTVhNGMtMTFlZi1hYWM1LWQxNGUxODBhZmRmNSIsImN1c3RvbWVySWQiOiIxMzgxNDAwMC0xZGQyLTExYjItODA4MC04MDgwODA4MDgwODAifQ.o87LyzShhh5RML5KHjQgUni-bf1zi3LJdMHpRfYTYGnUthj3H4x2edCnm6zj46UHX-R0F4uogMiV8kCe6SJJ3g',
                    }

                    params = (
                        ('keys', '' + type + ''),
                    )

                    response = requests.get('http://thingsboard.cloud/api/plugins/telemetry/DEVICE/' + device + '/values/timeseries', headers=headers, params=params)

                    if (type == "temperature"):
                        
                        headers_max_temp = {
                        'Content-Type': 'application/json',
                        'X-Authorization': 'Bearer eyJhbGciOiJIUzUxMiJ9.eyJzdWIiOiJ0b21hc3oua3Jva29zekBnbWFpbC5jb20iLCJ1c2VySWQiOiIxMTY0ZDU5MC01YTRjLTExZWYtYWFjNS1kMTRlMTgwYWZkZjUiLCJzY29wZXMiOlsiVEVOQU5UX0FETUlOIl0sInNlc3Npb25JZCI6IjQ4OTNjYjUzLTY1NTMtNGIwMC04OWEzLTZhODg4N2M1NjNhMSIsImV4cCI6MTcyNTU3OTM2OSwiaXNzIjoidGhpbmdzYm9hcmQuY2xvdWQiLCJpYXQiOjE3MjU1NTA1NjksImZpcnN0TmFtZSI6IlRvbWFzeiIsImxhc3ROYW1lIjoiS3Jva29zeiIsImVuYWJsZWQiOnRydWUsImlzUHVibGljIjpmYWxzZSwiaXNCaWxsaW5nU2VydmljZSI6ZmFsc2UsInByaXZhY3lQb2xpY3lBY2NlcHRlZCI6dHJ1ZSwidGVybXNPZlVzZUFjY2VwdGVkIjp0cnVlLCJ0ZW5hbnRJZCI6IjExMjZiYjIwLTVhNGMtMTFlZi1hYWM1LWQxNGUxODBhZmRmNSIsImN1c3RvbWVySWQiOiIxMzgxNDAwMC0xZGQyLTExYjItODA4MC04MDgwODA4MDgwODAifQ.o87LyzShhh5RML5KHjQgUni-bf1zi3LJdMHpRfYTYGnUthj3H4x2edCnm6zj46UHX-R0F4uogMiV8kCe6SJJ3g',
                        }

                        params_max_tem = (
                            ('keys', 'max_temperature'),
                        )
                        response_max = requests.get('http://thingsboard.cloud/api/plugins/telemetry/DEVICE/' + device + '/values/timeseries', headers=headers_max_temp, params=params_max_tem)
                        
                        params_default_tem = (
                            ('keys', 'default_temperature'),
                        )
                        response_default = requests.get('http://thingsboard.cloud/api/plugins/telemetry/DEVICE/' + device + '/values/timeseries', headers=headers_max_temp, params=params_default_tem)
                        

                        # display values
                        print(response.content)
                        print(response_max.content);
                        print(response_default.content);
                    
                        connection.sendall(response.content + b"_" + response_max.content)
                    else:
                        print(response.content)
                        connection.sendall(response.content)
                    break;
    finally:
        connection.close()