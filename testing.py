import torch
import train
from scipy.fft import fft, fftfreq
import matplotlib.pyplot as plt
import numpy as np
import models
import torchaudio

data = torch.load('./DATA/CUMIN-10.pt')

model = models.SpeechRec()
model.load_state_dict(torch.load('./speechmodel.pt'))
model.eval()

result = torch.argmax(model(data))
print(models.spice_translation_reverse[result])