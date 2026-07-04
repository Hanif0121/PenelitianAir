"""
Jalankan file ini dengan:
  python jalankan_server.py

Lalu buka browser ke:
  http://localhost:8000
"""
import http.server, webbrowser, os, threading

PORT = 8000
os.chdir(os.path.dirname(os.path.abspath(__file__)))

def open_browser():
    import time; time.sleep(1)
    webbrowser.open(f'http://localhost:{PORT}/index.html')

threading.Thread(target=open_browser, daemon=True).start()

print(f"Server jalan di http://localhost:{PORT}")
print("Tekan Ctrl+C untuk berhenti")
http.server.test(HandlerClass=http.server.SimpleHTTPRequestHandler, port=PORT, bind='')