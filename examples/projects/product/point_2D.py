from pyluos.services.service import Service
import numpy as np

class Point_2D(Service):

    # control modes
    _PLAY = 0
    _PAUSE = 1
    _STOP = 2

    # buffer modes
    _SINGLE = 0
    _CONTINUOUS = 1
    _STREAM = 2

    def __init__(self, id, alias, device):
        Service.__init__(self, 'Point_2D', id, alias, device)
        self._point = (0, 0)
        self._control = 0
        self._buffer_mode = _SINGLE
        self._sampling_freq = 100.0

    @property
    def position(self):
        return self._point

    @position.setter
    def position(self, new_val):
        data = []
        # check if the value is a tuple
        if isinstance(new_val, tuple):
            self._point = new_val
            data = list(new_val)
            self._push_value("linear_pos_2D", data)
        # check if new_val is a list
        elif hasattr(new_val, "__len__"):
            # If this is a list of tuples, put all the tuple couple of values on a list
            for i in range(len(new_val)):
                if isinstance(new_val[i], tuple):
                    data.append(new_val[i][0])
                    data.append(new_val[i][1])
            # send those values in an optimized way as a table of uint16_t
            self._push_data('linear_pos_2D', [len(data) * 2], np.array(data, dtype=np.uint16)) # multiplying by the size of uint16_t
        else:
            raise ValueError("Point_2D.position must be a list")

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
        self._push_value("time", 1.0 / sampling_freq)


    def _update(self, new_state):
        Service._update(self, new_state)
        if 'linear_pos_2D' in new_state.keys():
            self._point = new_state['linear_pos_2D']
