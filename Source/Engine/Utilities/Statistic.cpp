#include "Pch.h"

#include "Engine/Engine/FrameTime.h"
#include "Engine/Types/Math.h"
#include "Engine/Utilities/Statistic.h"

Array<Statistic*>* Statistic::s_statistics;
Statistic* Statistic::s_root;

Statistic::Statistic(const String& name)
	: m_name(name)
{
	// Folder/Group statistic, don't use directly.
}

Statistic::Statistic(const String& name, StatisticFrequency frequency, StatisticFormat format)
	: m_frequency(frequency)
	, m_format(format)
	, m_timer(0.0)
	, m_frameCount(0)
	, m_average(0.0)
	, m_min(0.0)
	, m_minTotal(0.0)
	, m_max(0.0)
	, m_maxTotal(0.0)
{
	if (s_statistics == nullptr)
	{
		s_statistics = new Array<Statistic*>();
	}
	s_statistics->push_back(this);

	// Insert into tree.
	Array<String> split = SplitString(name, "/");

	Statistic* root = GetRoot();
	for (int i = 0; i < split.size() - 1; i++)
	{
		String childName = split[i];
		Statistic* child = root->FindChild(childName);
		if (child == nullptr)
		{
			child = new Statistic(childName);
			root->AddChild(child);
		}
		root = child;
	}

	m_name = split[split.size() - 1];
	root->AddChild(this);
}

void Statistic::ResetForNewFrame(const FrameTime& time)
{
	m_frameCount++;
	m_averageTotal += m_value;
//	m_minTotal = Math::Min(m_value, m_minTotal);
//	m_maxTotal = Math::Max(m_value, m_maxTotal);
	m_min = Math::Min(m_value.load(), m_min);
	m_max = Math::Max(m_value.load(), m_max);

	m_timer += time.DeltaTime;
	if (m_timer >= 1.0f)
	{
		m_average = m_averageTotal / m_frameCount;
	//	m_min = m_minTotal;
	//	m_max = m_maxTotal;

	//	m_minTotal = m_value;
	//	m_maxTotal = m_value;
		m_averageTotal = 0.0f;
		m_frameCount = 0;
		m_timer = 0.0f;
	}

	if (m_frequency == StatisticFrequency::PerFrame)
	{
		m_value = {};
	}
}

void Statistic::Add(double value)
{
	while (true)
	{
		double original = m_value;
		double newValue = original + value;
		if (m_value.compare_exchange_strong(original, newValue))
		{
			return;
		}
	}
}

void Statistic::Set(double value)
{
	while (true)
	{
		double original = m_value;
		double newValue = value;
		if (m_value.compare_exchange_strong(original, newValue))
		{
			return;
		}
	}
}

String Statistic::GetName()
{
	return m_name;
}

StatisticFrequency Statistic::GetFrequency()
{
	return m_frequency;
}

StatisticFormat Statistic::GetFormat()
{
	return m_format;
}

double Statistic::GetValue()
{
	return m_value;
}

const Array<Statistic*>& Statistic::GetChildren()
{
	return m_children;
}

String Statistic::FormatValue(double value)
{
	switch (m_format)
	{
	case StatisticFormat::Integer:			return StringFormat("%i", (int)value);
	case StatisticFormat::Float:			return StringFormat("%.2f", value);
	case StatisticFormat::Bytes:			return FormatBytes((int)value);
	case StatisticFormat::Milliseconds:		return StringFormat("%.2f ms", value);
	}
	return StringFormat("%f", value);
}

String Statistic::GetFormattedValue()
{
	return FormatValue(m_value);
}

String Statistic::GetFormattedAverage()
{
	return FormatValue(m_average);
}

String Statistic::GetFormattedMin()
{
	return FormatValue(m_min);
}

String Statistic::GetFormattedMax()
{
	return FormatValue(m_max);
}

Statistic* Statistic::FindChild(const String& name)
{
	for (auto& child : m_children)
	{
		if (child->GetName() == name)
		{
			return child;
		}
	}
	return nullptr;
}

void Statistic::AddChild(Statistic* child)
{
	m_children.push_back(child);
}

const Array<Statistic*>& Statistic::GetStatistics()
{
	return *s_statistics;
}

Statistic* Statistic::GetRoot()
{
	if (s_root == nullptr)
	{
		s_root = new Statistic("");
	}
	return s_root;
}

void Statistic::NextFrame(const FrameTime& time)
{
	for (auto& stat : *s_statistics)
	{
		stat->ResetForNewFrame(time);
	}
}
