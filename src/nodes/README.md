# Node classes

A project consists of a root node and any number of child nodes. Each **Node** class is created by the **NodeCreator** class and will either be a UI element (widget) or an abstract object that modifies it's parent widget.

### Heirarchy

- **Node**: contains a created node that the user can modify
  - **NodeDeclaration**: contains declarations for properties, events, code generator, etc.
    - **NodeType**: Used to determine what children the node can create
    - **NodeCategory**: Categories of properties and events for PropertyGrid Panel


## NodeCreator

This class is initialized via XML files which declare each possible type of node along with their properties and events -- essentially everything needed to create the heirarchy of nodes that a project is made of.

### Node

This class is created by **NodeCreator** and can either be a UI widget, or an abstract object that modifies it's parent widget. A **Node** can contain any number of **NodeProperty** classes that can be modified by the user and an immutable **NodeDeclaration** class for accessing additional information and methods related to the specific widget the node contains.

#### NodeProperty

This class consists of a single string value which is used to store and retrieve the value from the project file. The string can be interpreted as several types of data -- methods are provided for converting as needed.

The class also contains a immutable **PropertyInfo** class which contains the name, default value, and help text of the property.

#### NodeDeclaration

This class contains immutable information about the widge the **Node** was created from.
