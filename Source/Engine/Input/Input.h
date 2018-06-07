#pragma once

class IInput
{
protected:
	IInput() { };

public:
	virtual ~IInput() { };
	virtual void Dispose() = 0;

};
