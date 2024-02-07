Class: UGameData
Author: Antoine Plouffe

Description: The UGameData class serves as a central hub for handling various game data
and functionalities for the application. It includes methods for loading instructions, checkpoints,
learn more content, and quiz questions. Additionally, it provides functions for populating UI elements, 
such as progress bars and quiz options, based on the loaded data. The class encapsulates error handling 
to display debug messages in case of data loading issues for rapid fixes.

The UGameData class employs a custom and specialized JSONhelper class that leverages polymorphism, adapting its behavior 
according to the specific structure type sought by UGameData.
