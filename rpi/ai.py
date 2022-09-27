import json
import numpy as np
import tflite_runtime.interpreter as tflite

class TFLiteYamnet():
    def __init__(self, path_to_yamnet):
        self.interpreter = tflite.Interpreter(path_to_yamnet)
        self.input_details = self.interpreter.get_input_details()
        self.waveform_input_index = self.input_details[0]['index']

        self.output_details = self.interpreter.get_output_details()
        self.scores_output_index = self.output_details[0]['index']
        self.embeddings_output_index = self.output_details[1]['index']
        self.spectrogram_output_index = self.output_details[2]['index']

    
    def __call__(self, waveform):
        self.interpreter.resize_tensor_input(
            self.waveform_input_index, [len(waveform)], strict=True
        )
        self.interpreter.allocate_tensors()
        self.interpreter.set_tensor(self.waveform_input_index, waveform)
        self.interpreter.invoke()

        embeddings = self.interpreter.get_tensor(self.embeddings_output_index)
        scores = self.interpreter.get_tensor(self.scores_output_index)
        self.interpreter.reset_all_variables()
        
        labels = scores.argmax(axis=-1)
        return embeddings, labels

class TFLiteToplayer():
    def __init__(self, path_to_toplayer):
        self.interpreter = tflite.Interpreter(path_to_toplayer)
        self.input_details = self.interpreter.get_input_details()
        self.embeddings_input_index = self.input_details[0]["index"]

        self.output_details = self.interpreter.get_output_details()
        self.cats_output_index = self.output_details[0]["index"]
        self.logits_output_index = self.output_details[1]["index"]
    
    def __call__(self, embeddings):
        self.interpreter.allocate_tensors()
        self.interpreter.set_tensor(self.embeddings_input_index, embeddings)
        self.interpreter.invoke()

        cats = self.interpreter.get_tensor(self.cats_output_index)
        logits = self.interpreter.get_tensor(self.logits_output_index)
        self.interpreter.reset_all_variables()
        return cats, logits

class AI():
    def __init__(self, 
                 yamnet_fpath='yamnet.tflite', 
                 toplayer_fpath='toplayer.tflite', 
                 metadata_fpath='metadata.json'):

        # Load pre-requisite stuff
        self.yamnet = TFLiteYamnet(yamnet_fpath)
        self.toplayer = TFLiteToplayer(toplayer_fpath)

        with open(metadata_fpath, 'r') as f:
            self.metadata = json.load(f)

        self.categories = self.metadata['categories']
        self.cat2class = self.metadata['cat-class']
        self.yamlabs = self.metadata['yamnet-labels']
    
    def __call__(self, waveform, printf=lambda x:print(x)):
        embed, labels = self.yamnet(waveform)
        [cat_idx], _ = self.toplayer(embed)
        cat = self.categories[cat_idx]

        printf(f"Category (`{cat}`) under Class (`{self.cat2class[cat]}`)")
        # Should say Category (`wind`) under Class (`natural_ambient`)

        return cat, self.cat2class[cat], [self.yamlabs[i] for i in labels]
    
    def randomwav(self):
        # Test Input: 5 seconds of noise (simulating mono 16 kHz waveform samples)
        return np.random.rand(5 * 16000).astype(np.float32)
