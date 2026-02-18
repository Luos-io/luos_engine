from pyluos.services.service import Service
import numpy as np

class Power(Service):

    # control modes
    _PLAY = 0
    _PAUSE = 1
    _STOP = 2

    # buffer modes
    _SINGLE = 0
    _CONTINUOUS = 1
    _STREAM = 2

    def __init__(self, id, alias, device):
        Service.__init__(self, 'Power', id, alias, device)
        self._power = 0.0
        self._control = 0
        self._buffer_mode = self._SINGLE
        self._sampling_freq = 100.0
        self._value = 0.0

    @property
    def power_ratio(self):
        self._value

    @power_ratio.setter
    def power_ratio(self, new_val):
        data = []
        # check if new_val is a list
        if hasattr(new_val, "__len__"):
            # If this is a list , put all the tuple couple of values on a list
            for i in range(len(new_val)):
                data.append(min(max(new_val[i], 0.0), 100.0))
            # send those values in an optimized way as a table of uint16_t
            self._push_data('power_ratio', [len(data) * 4], np.array(data, dtype=np.float32))
        else:
            new_val = min(max(new_val, 0.0), 100.0)
            self._value = new_val
            self._push_value("power_ratio",new_val)

    def play(self):
        self._control = self._PLAY
        self._push_value('control', self._control)

    def pause(self):
        self._control = self._PAUSE
        self._push_value('control', self._control)

    def stop(self):
        self._control = self._STOP
        self._push_value('control', self._control)

    def single(self):
        self._buffer_mode = self._SINGLE
        self._push_value('buffer_mode', self._buffer_mode)

    def continuous(self):
        self._buffer_mode = self._CONTINUOUS
        self._push_value('buffer_mode', self._buffer_mode)
    
    def stream(self):
        self._buffer_mode = self._STREAM
        self._push_value('buffer_mode', self._buffer_mode)

    @property
    def sampling_freq(self):
        return self._sampling_freq

    @sampling_freq.setter
    def sampling_freq(self, sampling_freq):
        self._sampling_freq = sampling_freq
        self._push_value("sampling_freq", sampling_freq)


    def _update(self, new_state):
        Service._update(self, new_state)
        if 'linear_pos_2D' in new_state.keys():
            self._point = new_state['linear_pos_2D']
