import torch
import torchaudio


class AudioBuffer():
    def __init__(self, chunk_size) -> None:
        self.waveform = torch.zeros(8000)
        self.chunk_size = chunk_size

    def add_chunk(self, chunk : list):
        self.waveform[self.chunk_size:] = self.waveform[:-self.chunk_size].clone()
        self.waveform[:self.chunk_size] = torch.Tensor(chunk)
    
    def get_scaled_tensor(self):
        """Returns tensor upsampled from 8khz to 16khz"""
        return torchaudio.functional.resample(torch.unsqueeze(self.waveform, 0), 8000, 16000)

    def get_raw_tensor(self):
        return self.waveform