#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Types/Dictionary.h"

struct FrameTime;

enum class StatisticFrequency
{
	PerFrame,
	Persistent
};

enum class StatisticFormat
{
	Integer,
	Float,
	Bytes,
	Milliseconds
};

struct Statistic 
{
private:	
	static Array<Statistic*>* s_statistics;
	static Statistic* s_root;

	String m_name;
	StatisticFrequency m_frequency;
	StatisticFormat m_format;

	Array<Statistic*> m_children;

	std::atomic<double> m_value;

	double m_timer;
	int m_frameCount;

	double m_averageTotal;
	double m_average;
	double m_minTotal;
	double m_min;
	double m_maxTotal;
	double m_max;

protected:
	Statistic(const String& name);

	String FormatValue(double value);

public:
	Statistic(const String& name, StatisticFrequency frequency, StatisticFormat format);

	void ResetForNewFrame(const FrameTime& time);
	void Add(double value);
	void Set(double value);

	String GetName();
	StatisticFrequency GetFrequency();
	StatisticFormat GetFormat();
	const Array<Statistic*>& GetChildren();

	double GetValue();
	String GetFormattedValue();
	String GetFormattedAverage();
	String GetFormattedMin();
	String GetFormattedMax();

	Statistic* FindChild(const String& name);
	void AddChild(Statistic* child);

	static const Array<Statistic*>& GetStatistics();
	static Statistic* GetRoot();
	static void NextFrame(const FrameTime& time);
};