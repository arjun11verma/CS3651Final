import models
import torch
from torch.utils.data import Dataset, DataLoader
import os

class SpiceDataset(Dataset):
    def __init__(self) -> None:
        super().__init__()
        self.base_path = './DATA/'
        self.filenames = os.listdir(self.base_path)
    
    def __len__(self):
        return len(self.filenames)

    def __getitem__(self, idx):
        filename = self.base_path + self.filenames[idx]
        data = torch.load(filename)
        label = self.filenames[idx].split('-')[0]
        return data, label

def train(model : models.SpeechRec, dataset : SpiceDataset, lr, epochs):
    model.train()
    for parameter in model.encoder_model.parameters():
        parameter.requires_grad = False

    loader = DataLoader(dataset, 1, shuffle=True)
    optimizer = torch.optim.Adam(model.parameters(), lr)

    for i in range(epochs):
        data, label = next(iter(loader))
        label = label[0]
        data = torch.squeeze(data, dim=0)

        optimizer.zero_grad()
        output = model(data)
        
        one_hot_vector = torch.zeros(8)
        one_hot_vector[models.spice_translation[label]] = -1
        loss = torch.dot(output, one_hot_vector)

        loss.backward()
        optimizer.step()

        print(f'Running loss: {loss.item()}')
    
    torch.save(model.state_dict(), './speechmodel.pt')

if __name__ == '__main__':
    dataset = SpiceDataset()
    model = models.SpeechRec()

    train(model, dataset, 0.000125, 10000)