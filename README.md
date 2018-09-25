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
}

int main() {
    Config myConfig(*ConfigMessage);
	Config myConfig(*ConfigMessage);
	std::string outVal, outVal1, outVal2;
	myConfig.GetLastSeen("AstroX", outVal);
	myConfig.GetLastSeen("AstroX2", outVal1);
	myConfig.GetLastSeen("AstroX3", outVal2);
	myConfig.SetLastSeen("AstroX"); //Saves time(now) to the config

	if (outVal == "") //note were looking at original AstroX data
	{
		std::cout << "AstroX: has not been seen until now." << std::endl;
	} 
	else {
		std::cout << "AstroX: was last seen " << outVal.c_str() << " ago." << std::endl;
	}

    system("pause");

    return TRUE;
}
```
