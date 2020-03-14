import aiohttp
import asyncio
import async_timeout
import math

from urllib.parse import urljoin

from .error import StateOutdated


class LEDStripe:
    """
    Represents and control an Ohm-LED device.
    """

    def __init__(self, baseURL, timeout=5):
        """
        Instanciates an Ohm-LED device.

        :param baseURL: The base URL for the device.
        :param timeout: The maximum time to wait for, in seconds for state updates.
        """
        self.baseURL = baseURL
        self.timeout = timeout

    def _url(self, path):
        return urljoin(self.baseURL, path)

    async def get_state(self):
        """
        Get the state of the device.

        :return: The new state dict.
        """
        async with aiohttp.ClientSession() as session:
            async with async_timeout.timeout(self.timeout):
                async with session.get(self._url('/v1/state/')) as response:
                    return await response.json()

    async def set_state(self, state):
        """
        Set the state of the device from a raw state dict.

        :param state: The state dict. None values are ignored.
        :return: The new state dict.
        """
        state = {k: v for k, v in state.items() if v is not None}

        async with aiohttp.ClientSession() as session:
            async with async_timeout.timeout(self.timeout):
                async with session.put(self._url('/v1/state/'), json=state) as response:
                    if response.status == 409:
                        raise StateOutdated(state=await response.json())

                    return await response.json()

    async def off(self):
        """
        Turn the LED stripe off completely.

        :return: The new state dict.
        """
        return await self.set_state({"mode": "off"})

    async def on(self, hsv=None):
        """
        Turn the LED stripe on.

        :param hsv: If specified, a triplet of (hue, saturation, value).
        :return: The new state dict.
        """
        state = {"mode": "on"}

        if hsv is not None:
            state["hue"], state["saturation"], state["value"] = hsv

        return await self.set_state(state)

    async def pulse(self, hsv=None, period=None, easing=None):
        """
        Turn the LED stripe in pulse mode.

        :param hsv: If specified, a triplet of (hue, saturation, value).
        :param period: If specified, the period in seconds of the pulse animation.
        :param easing: If specified, the easing function to use for animations.
        :return: The new state dict.
        """
        state = {"mode": "pulse"}

        if hsv is not None:
            state["hue"], state["saturation"], state["value"] = hsv

        if period is not None:
            state["period"] = math.floor(period * 1000)

        if easing is not None:
            state["easing"] = easing

        return await self.set_state(state)

    async def rainbow(self):
        """
        Turn the LED stripe in rainbow mode.

        :return: The new state dict.
        """
        return await self.set_state({"mode": "rainbow"})

    async def balls(self, period=None, num_balls=None, easing=None):
        """
        Turn the LED stripe in balls mode.

        :param period: If specified, the period in seconds of the balls animation.
        :param num_balls: If specified, the number of balls.
        :param easing: If specified, the easing function to use for animations.
        :return: The new state dict.
        """
        state = {"mode": "balls"}

        if period is not None:
            state["period"] = math.floor(period * 1000)

        if num_balls is not None:
            state["num-balls"] = num_balls

        if easing is not None:
            state["easing"] = easing

        return await self.set_state(state)

    async def knight_rider(self, hsv=None, period=None, easing=None):
        """
        Turn the LED stripe in knight-rider mode.

        :param hsv: If specified, a triplet of (hue, saturation, value).
        :param period: If specified, the period in seconds of the knight-rider animation.
        :param easing: If specified, the easing function to use for animations.
        :return: The new state dict.
        """
        state = {"mode": "knight-rider"}

        if hsv is not None:
            state["hue"], state["saturation"], state["value"] = hsv

        if period is not None:
            state["period"] = math.floor(period * 1000)

        if easing is not None:
            state["easing"] = easing

        return await self.set_state(state)

    async def fire(self, fire_cooling=None, fire_sparking=None):
        """
        Turn the LED stripe in fire mode.

        :param fire_cooling: If specified, the fire cooling factor.
        :param fire_sparking: If specified, the fire sparking factor.
        :return: The new state dict.
        """
        state = {"mode": "fire"}

        if fire_cooling is not None:
            state["fire-cooling"] = fire_cooling

        if fire_sparking is not None:
            state["fire-sparking"] = fire_sparking

        return await self.set_state(state)