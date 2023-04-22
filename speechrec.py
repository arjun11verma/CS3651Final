import torch
import torchaudio

# 825 is a good cutoff for speech

class AudioBuffer():
    def __init__(self, chunk_size, buffer_size) -> None:
        self.waveform = torch.zeros(buffer_size)
        self.buffer_size = buffer_size
        self.chunk_size = chunk_size

        self.speech_enum = 0 # 0 means no peak detected, 1 means peak detected
        self.chunks_inbetween = 0
        self.is_speech = False

    def add_chunk(self, chunk : list):
        chunk = torch.Tensor(chunk)

        self.waveform[:-self.chunk_size] = self.waveform[self.chunk_size:].clone()
        self.waveform[-self.chunk_size:] = chunk

        max_val = torch.max(chunk)
        if (self.speech_enum == 0 and max_val > 850):
            self.speech_enum = 1
            self.chunks_inbetween = 0
        elif (self.speech_enum == 1 and max_val > 850):
            self.chunks_inbetween += 1
        elif (self.speech_enum == 1 and max_val < 625):
            self.speech_enum = 0
            if (self.chunks_inbetween > 1): self.is_speech = True
    
    def get_scaled_tensor(self):
        """Returns tensor upsampled from 8khz to 16khz"""
        return torchaudio.functional.resample(torch.unsqueeze(self.waveform, 0), self.buffer_size, 16000)

    def get_raw_tensor(self):
        return self.waveform

    def reset_speech(self):
        self.is_speech = False