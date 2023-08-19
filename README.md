# Cart and Pole

This project is cart and pole Q-learning representation consisting of UDP client as a c++ unreal preject and a python sever.

The [client](./Source/CartAndPole/Public/Connection/UdpClient.h) has a [mechanism actor](./Source/CartAndPole/Public/Mechanism/CartPoleMechanism.h) (cart and pole movable meshes) and actor components for [training](./Source/CartAndPole/Public/Mechanism/MovementInput.h) and [predicting](./Source/CartAndPole/Public/Mechanism/NneModelComponent.h) using a loaded ONNX model mesh.

The [server](./TrainigServer/) runs with asynncio upd server usinf tensorflow for the Q-learning. To run it:

1. Activate venv (Windows):

```batch
.\venv\Scripts\activate
```

2. install dependencies:

```batch
pip install -r requirements.txt
```

3. Run the server:

```batch
python main.py
```
