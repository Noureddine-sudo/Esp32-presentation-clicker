import pyautogui
from flask import Flask
from zeroconf import ServiceInfo, Zeroconf
import socket
import time

app = Flask(__name__)

@app.route('/next')
def next_slide():
    pyautogui.press('right')
    print(">>> NEXT slide")
    return "OK", 200

@app.route('/prev')
def prev_slide():
    pyautogui.press('left')
    print(">>> PREV slide")
    return "OK", 200

def start_mdns():
    zeroconf = Zeroconf()
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(("8.8.8.8", 80))
        local_ip = s.getsockname()[0]
    except:
        local_ip = "127.0.0.1"
    finally:
        s.close()

    info = ServiceInfo(
        "_http._tcp.local.",
        "serveur-pres._http._tcp.local.",
        addresses=[socket.inet_aton(local_ip)],
        port=5000,
        server="serveur-pres.local.",
        properties={"path": "/"}
    )
    
    print(f"🚀 Broadcasting as 'serveur-pres.local' on IP: {local_ip}")
    zeroconf.register_service(info)
    return zeroconf, info

if __name__ == "__main__":
    zc, info = start_mdns()
    print("Waiting 5 seconds for mDNS to propagate...")
    time.sleep(5)                    # ← this fixes most "server not found"
    
    try:
        app.run(host='0.0.0.0', port=5000, debug=False, threaded=True)
    except KeyboardInterrupt:
        pass
    finally:
        print("Unregistering mDNS...")
        zc.unregister_service(info)
        zc.close()
