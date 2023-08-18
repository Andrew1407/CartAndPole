import asyncio

from qlearning import LearningMetrics
from plots import PlotGenerator
from action_trainer import ActionTrainer
from upd_protocol import QLearningUdpProtocol


HOST = '127.0.0.1'
PORT = 8080

BATCH_SIZE = 256
GAMMA = .999
EPS_START = 1
EPS_END = .01
EPS_DECAY = .001

TARGET_MODEL_UPDATE = 10
MEMORY_SIZE = 100_000
LEARNIING_RATE = .001
DISPLAY_PLOTS = True


def make_variables() -> tuple[LearningMetrics, PlotGenerator, ActionTrainer]:
    learning_metrics = LearningMetrics()
    plot_generator = PlotGenerator(learning_metrics=learning_metrics, title='Training', display=DISPLAY_PLOTS)
    action_trainer = ActionTrainer(replay_memory_size=MEMORY_SIZE, batch_size=BATCH_SIZE,
                                   learning_rate=LEARNIING_RATE, gamma=GAMMA, eps=(EPS_START, EPS_END, EPS_DECAY))
    return learning_metrics, plot_generator, action_trainer


async def main():
    learning_metrics, plot_generator, action_trainer = make_variables()
    loop = asyncio.get_event_loop()
    server_protocol = QLearningUdpProtocol(learning_metrics=learning_metrics, action_trainer=action_trainer,
                                           plot_generator=plot_generator, weights_update_rate=TARGET_MODEL_UPDATE)
    print(f"UDP server is starting on udp://{HOST}:{PORT}")
    transport, protocol = await loop.create_datagram_endpoint(
        protocol_factory=lambda: server_protocol,
        local_addr=(HOST, PORT)
    )
    try:
        await asyncio.Future()  # Run forever
    finally:
        transport.close()


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("Server is shutting down...")
