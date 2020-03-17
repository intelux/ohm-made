import aiohttp
import asyncio
import async_timeout
import math

from urllib.parse import urljoin

from .error import StateOutdated


class Device:
    """
    Represents and control an Ohm-LED device.
    """

    def __init__(self, base_url, timeout=5):
        """
        Instanciates an Ohm-LED device.

        :param base_url: The base URL for the device.
        :param timeout: The maximum time to wait for, in seconds for state updates.
        """
        self.base_url = base_url
        self.timeout = timeout

    def __repr__(self):
        return 'Device(base_url=%s, timeout=%d)' % (self.base_url, self.timeout)

    def _url(self, path):
        return urljoin(self.base_url, path)

    async def get_info(self):
        """
        Get info on the device.

        :return: The info dict.
        """
        async with aiohttp.ClientSession() as session:
            async with async_timeout.timeout(self.timeout):
                async with session.get(self._url('/v1/info/')) as response:
                    return await response.json()

    async def get_state(self):
        """
        Get the state of the device.

        :return: The new state dict.
        """
        async with aiohttp.ClientSession() as session:
            async with async_timeout.timeout(self.timeout):
                async with session.get(self._url('/v1/state/')) as response:
                    return await response.json()

    async def set_state(self, state={}, **kwargs):
        """
        Set the state of the device from a raw state dict.

        :param state: The state dict. None values are ignored.
        :param hsv: If specified, a triplet of (hue, saturation, value).
        :param period: If specified, the period in seconds of the pulse animation.
        :param easing: If specified, the easing function to use for animations.
        :param num_balls: If specified, the number of balls.
        :param fire_cooling: If specified, the fire cooling factor.
        :param fire_sparking: If specified, the fire sparking factor.
        :return: The new state dict.
        """
        hsv = kwargs.pop('hsv', None)
        period = kwargs.pop('period', None)
        easing = kwargs.pop('easing', None)
        fire_cooling = kwargs.pop('fire_cooling', None)
        fire_sparking = kwargs.pop('fire_sparking', None)

        if hsv is not None:
            kwargs["hue"], kwargs["saturation"], kwargs["value"] = hsv

        if period is not None:
            kwargs["period"] = math.floor(period * 1000)

        if easing is not None:
            kwargs["easing"] = easing

        if fire_cooling is not None:
            state["fire-cooling"] = fire_cooling

        if fire_sparking is not None:
            state["fire-sparking"] = fire_sparking

        state.update({k: v for k, v in kwargs.items() if v is not None})
        state = {k: v for k, v in state.items() if v is not None}

        async with aiohttp.ClientSession() as session:
            async with async_timeout.timeout(self.timeout):
                async with session.put(self._url('/v1/state/'), json=state) as response:
                    if response.status == 409:
                        raise StateOutdated(state=await response.json())

                    return await response.json()

    async def off(self, **kwargs):
        """
        Turn the LED stripe off completely.

        Any of the named arguments from `set_state` can also be used here.

        :return: The new state dict.
        """
        return await self.set_state(mode='off', **kwargs)

    async def on(self, **kwargs):
        """
        Turn the LED stripe on.

        Any of the named arguments from `set_state` can also be used here.

        :return: The new state dict.
        """
        return await self.set_state(mode='on', **kwargs)

    async def pulse(self, **kwargs):
        """
        Turn the LED stripe in pulse mode.

        Any of the named arguments from `set_state` can also be used here.

        :return: The new state dict.
        """
        return await self.set_state(mode='pulse', **kwargs)

    async def colorloop(self, **kwargs):
        """
        Turn the LED stripe in colorloop mode.

        Any of the named arguments from `set_state` can also be used here.

        :return: The new state dict.
        """
        return await self.set_state(mode='colorloop', **kwargs)

    async def rainbow(self, **kwargs):
        """
        Turn the LED stripe in rainbow mode.

        Any of the named arguments from `set_state` can also be used here.

        :return: The new state dict.
        """
        return await self.set_state(mode='rainbow', **kwargs)

    async def knight_rider(self, **kwargs):
        """
        Turn the LED stripe in knight-rider mode.

        Any of the named arguments from `set_state` can also be used here.

        :return: The new state dict.
        """
        return await self.set_state(mode='knight-rider', **kwargs)

    async def fire(self, **kwargs):
        """
        Turn the LED stripe in fire mode.

        Any of the named arguments from `set_state` can also be used here.

        :return: The new state dict.
        """
        return await self.set_state(mode='fire', **kwargs)
