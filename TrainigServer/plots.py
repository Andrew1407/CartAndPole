import matplotlib
import matplotlib.pyplot as plt

import numpy as np
import io
import base64

from qlearning import LearningMetrics


class PlotGenerator:
    def __init__(self, learning_metrics: LearningMetrics, title: str, display: bool = True):
        self.set_display_mode(display)
        self.__learning_metrics = learning_metrics
        self.__title = title

        plt.ion()
        self.__fig, self.__ax = plt.subplots()
        self.__line, = self.__ax.plot([], [])
        self.__ax.set_xlabel('Episodes')
        self.__ax.set_ylabel('Duration')
        self.update_plot()


    def set_display_mode(self, display: bool):
        matplotlib.use('TkAgg' if display else 'Agg')


    def update_plot(self):
        title = '%s (episode: %d/%d)' % (self.__title, self.__learning_metrics.current_episode + 1, self.__learning_metrics.episodes_total)
        self.__ax.set_title(title)
        episodes = self.__learning_metrics.episodes_duration

        if episodes is None:
            self.__line.set_data([], [])
        else:
            bound = self.__learning_metrics.current_episode + 1
            if bound > len(self.__learning_metrics.episodes_duration):
                return
            y = self.__learning_metrics.episodes_duration[:bound]
            x = np.arange(len(y))
            self.__line.set_data(x, y)

        self.__ax.relim()
        self.__ax.autoscale_view()
        plt.pause(0.01)


    def base64(self) -> str:
        buffer = io.BytesIO()
        self.__fig.savefig(buffer, format='png')
        buffer.seek(0)  # Rewind the buffer to the beginning
        base64_string = base64.b64encode(buffer.read()).decode()
        return base64_string
