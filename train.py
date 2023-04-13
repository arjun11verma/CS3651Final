import speechrec
import torch
import torchaudio

import time

if __name__ == '__main__':
    buffer = speechrec.AudioBuffer(400)
    for i in range(20):
        buffer.add_chunk([j for j in range(400)])

    model = torchaudio.pipelines.WAV2VEC2_ASR_BASE_960H.get_model()

    start = time.time()
    emission, _ = model(buffer.get_scaled_tensor())
    print(emission) # time is pretty good, every four updates or so we can run speech recognition