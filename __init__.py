import aiohttp
import asyncio
import async_timeout

from urllib.parse import urljoin


class LEDStripe:
    def __init__(self, baseURL, timeout=5):
        self.baseURL = baseURL
        self.timeout = timeout

    async def get_state(self):
        async with aiohttp.ClientSession() as session:
            async with async_timeout.timeout(self.timeout):
                async with session.get(self._url('/v1/state/')) as response:
                    return await response.json()

    async def set_state(self, state):
        async with aiohttp.ClientSession() as session:
            async with async_timeout.timeout(self.timeout):
                async with session.put(self._url('/v1/state/'), json=state) as response:
                    return await response.json()

    async def off(self):
        return await self.set_state({"mode": "off"})

    async def on(self):
        return await self.set_state({"mode": "on"})

    def _url(self, path):
        return urljoin(self.baseURL, path)


async def main():
    led = LEDStripe("http://192.168.0.191")
    print(await led.off())
    await asyncio.sleep(1)
    print(await led.on())


if __name__ == "__main__":
    loop = asyncio.get_event_loop()
    loop.run_until_complete(main())