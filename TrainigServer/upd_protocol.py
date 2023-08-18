import os
import numpy as np
from asyncio import DatagramProtocol, DatagramTransport

from  qlearning import LearningMetrics
from action_trainer import ActionTrainer
from plots import PlotGenerator


def parse_inputs(data: str) -> tuple[list[float], list[float], int, int, bool, int]:
    args = (i for i in data.split(' '))
    state = [float(next(args)) for _ in range(4)]
    next_state = [float(next(args)) for _ in range(4)]
    action = int(next(args))
    done = int(next(args)) == 1
    current_step = int(next(args))
    episode = int(next(args))
    return state, next_state, action, done, current_step, episode


class QLearningUdpProtocol(DatagramProtocol):
    def __init__(self, learning_metrics: LearningMetrics, action_trainer: ActionTrainer,
                 plot_generator: PlotGenerator, weights_update_rate: int, plot_update_rate: int = 1):
        self.transport: DatagramTransport = None

        self.__weights_update_rate = weights_update_rate
        self.__plot_update_rate = plot_update_rate

        self.__learning_metrics = learning_metrics
        self.__action_trainer = action_trainer
        self.__plot_generator = plot_generator
        
        self.__healthcheck_cmds: tuple[str, ...] = ('heartbeat', 'healthcheck')
        self.__sample_title = 'sample: '
        self.__action_title = 'action: '
        self.__training_params_title = 'training_params: '


    def connection_made(self, transport: DatagramTransport):
        self.transport = transport


    def datagram_received(self, data: bytes, addr: tuple[str, int]):
        message = data.decode()
        if message in self.__healthcheck_cmds:
            response = message + ': ok'
            self.transport.sendto(response.encode(), addr)
        elif message.startswith(self.__training_params_title):
            response = self.__set_training_params(message)
            self.transport.sendto(response.encode(), addr)
        elif message.startswith(self.__action_title):
            if 'nan(ind)' in message: return
            response = self.__get_action(message)
            self.transport.sendto(response.encode(), addr)
        elif message.startswith(self.__sample_title):
            if 'nan(ind)' in message: return
            response = self.__take_sample(message)
            self.transport.sendto(response.encode(), addr)
        elif message == 'save_model':
            self.__save_models()
            response = 'save_model: ok'
            self.transport.sendto(response.encode(), addr)
        elif message == 'get_plot':
            response = self.__plot_generator.base64()
            self.transport.sendto(response.encode(), addr)
        else:
            response = 'unknown command'
            self.transport.sendto(response.encode(), addr)
    

    def __save_models(self):
        storage_dir = 'models/'
        if not os.path.exists(storage_dir): os.mkdir(storage_dir)
        policy_path = '%spolicy_%d' % (storage_dir, self.__learning_metrics.episodes_total)
        target_path = '%starget_%d' % (storage_dir, self.__learning_metrics.episodes_total)
        self.__action_trainer.save_models(policy=policy_path, target=target_path)
    

    def __set_training_params(self, message: str) -> str:
        data_raw = message[len(self.__training_params_title):]
        episodes, max_actions = tuple(int(p) for p in data_raw.split(' '))
        self.__learning_metrics.episodes_total = episodes
        self.__learning_metrics.actions_limit = max_actions
        self.__learning_metrics.episodes_duration = np.zeros(episodes)
        return self.__training_params_title + 'ok'


    def __take_sample(self, message: str) -> str:
        data_raw = message[len(self.__sample_title):]
        state, next_state, action, done, current_step, episode = parse_inputs(data_raw)
        reward = 0 if done else 1
        self.__learning_metrics.current_episode = episode

        if done:
            if current_step > 0 and current_step % self.__plot_update_rate == 0:
                self.__plot_generator.update_plot()
            if current_step > 0 and current_step % self.__weights_update_rate == 0:
                self.__action_trainer.update_model_parameters()

        self.__action_trainer.add_sample(state=state, action=action, next_state=next_state, reward=reward, done=done)
        self.__action_trainer.train_by_sampple()
        return self.__sample_title + 'ok'


    def __get_action(self, message: str) -> str:
        data_raw = message[len(self.__action_title):]
        state = [ float(i) for i in data_raw.split(' ') ]
        episode = int(state.pop())
        current_step = int(state.pop())
        action = self.__action_trainer.get_action(current_step, state)
        self.__learning_metrics.episodes_duration[episode] += 1
        return self.__action_title + str(action)
