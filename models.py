import torch
import torch.nn as nn
import torchaudio

spice_translation = {
    'BASIL': 0,
    'CUMIN': 1,
    'THYME': 2,
    'MINT': 3,
    'SALT': 4,
    'CHILI': 5,
    'DILL' : 6,
    'MACE' : 7
}

spice_translation_reverse = [
    'BASIL',
    'CUMIN',
    'THYME',
    'MINT',
    'SALT',
    'CHILI',
    'DILL',
    'MACE'
]

class SpeechRec(nn.Module):
    def __init__(self, *args, **kwargs) -> None:
        super().__init__(*args, **kwargs)

        self.encoder_model = torchaudio.pipelines.WAV2VEC2_ASR_BASE_960H.get_model()
        for parameter in self.encoder_model.parameters():
            parameter.requires_grad = False # freeze encoder

        self.classifier_head_one = nn.Sequential(
            nn.Linear(768, 350),
            nn.Dropout(0.25),
            nn.ReLU(),
            nn.Linear(350, 1),
            nn.ReLU()
        )

        self.classifier_head_two = nn.Sequential(
            nn.Linear(588, 200),
            nn.Dropout(0.25),
            nn.ReLU(),
            nn.Linear(200, 8),
            nn.LogSoftmax(dim=0)
        )
    
    def forward(self, x):
        x = self.encoder_model.extract_features(x)
        x = torch.stack(x[0])
        x = torch.squeeze(x, 1)
        x = self.classifier_head_one(x)
        x = torch.flatten(x)
        x = self.classifier_head_two(x)
        return x