import tensorflow as tf
import tf2onnx

import random
import qlearning as ql


class ActionTrainer:
    def __init__(self, replay_memory_size: int, batch_size: int, learning_rate: float, gamma: float, eps: tuple[float, float, float]):
        self.__batch_size = batch_size
        self.__gamma = gamma
        self.__memory = ql.ReplayMemory(capacity=replay_memory_size)
        eps_start, eps_end, epd_decay = eps
        self.__epsilon_strategy = ql.EpsilonGreedyStrategy(start=eps_start, end=eps_end, decay=epd_decay)
        self.__optimizer = tf.keras.optimizers.Adam(learning_rate=learning_rate)

        self.__policy_net = ql.create_q_dnn_model()
        self.__target_net = ql.create_q_dnn_model()
        self.update_model_parameters()
        self.__target_net.trainable = False
    

    def update_model_parameters(self):
        self.__target_net.set_weights(self.__policy_net.get_weights())


    def get_action(self, current_step: int, state: list[float]) -> int:
        if self.__epsilon_strategy(current_step) > random.random():
            action = random.randrange(2)
        else:
            state_tensor = tf.convert_to_tensor([state])
            action = self.__policy_net(state_tensor).numpy().argmax(axis=1)[0]
        return action


    def add_sample(self, state: list[float], action: int, next_state: list[float], reward: int, done: bool):
        self.__memory.push(ql.Experience(state, action, next_state, reward, done))


    def train_by_sampple(self) -> bool:
        can_provide_sample = self.__memory.can_provide_sample(self.__batch_size)
        if not can_provide_sample: return False
        
        with tf.GradientTape() as tape:
            experiences = self.__memory.sample(self.__batch_size)
            states, actions, rewards, next_states, valid = ql.extract_tensors(experiences)
            actions = tf.expand_dims(actions, axis=-1)
            current_q_values = tf.gather(self.__policy_net(states), actions, batch_dims=1)
            next_q_values = ql.get_next_q_max(self.__target_net, next_states, valid)
            target_q_values = (next_q_values * self.__gamma) + rewards

            loss = tf.keras.losses.MeanSquaredError()(current_q_values, tf.expand_dims(target_q_values, axis=-1))

        gradients = tape.gradient(loss, self.__policy_net.trainable_variables)
        self.__optimizer.apply_gradients(zip(gradients, self.__policy_net.trainable_variables))

        return True


    def save_models(self, policy: str, target: str):
        self.__target_net.save(target)
        self.__policy_net.save(policy)
        tf2onnx.convert.from_keras(model=self.__target_net, output_path=target + '.onnx')
        tf2onnx.convert.from_keras(model=self.__policy_net, output_path=policy + '.onnx')
