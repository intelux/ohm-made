import asyncio
import socket

from aiozeroconf import ServiceBrowser, Zeroconf

from .device import Device


async def scan_devices(timeout=5):
    """
    Scan for devices on the network.

    :return: A list of devices found.
    """
    loop = asyncio.get_running_loop()
    zeroconf = Zeroconf(loop)
    devices = []

    def on_service_state_change(zc, service_type, name, change):
        host = name.split(".")[0] + ".local"
        host = socket.gethostbyname(host)
        device = Device(base_url=f"http://{host}")
        devices.append(device)

    ServiceBrowser(zeroconf, "_ohm-led._tcp.local.", handlers=[on_service_state_change])

    await asyncio.sleep(timeout)
    await zeroconf.close()

    return devices