# ConfigurationClass
Configuration Class
---
If you so want to use the function event method feel free.
```
void ConfigMessage(MessageTypes MessageType, const char sMessageOut[])
{
    switch (MessageType) {
        case MessageTypes::Error : {
            std::cout << "Error: " << sMessageOut << std::endl;
            break;
        }
        case MessageTypes::Warning : {
            std::cout << "Warning: " << sMessageOut << std::endl;
            break;
        }
        case MessageTypes::Good : {
            std::cout << "Good: " << sMessageOut << std::endl;
            break;
        }
    }
    //std::cout << speedtime.count() << "ms.\r\n";
}

int main() {
    Config myConfig(*ConfigMessage);

    system("pause");

    return TRUE;
}
```
