#pragma once
#include "graphics.hpp"

#include <map>
#include <memory>
#include <string>

namespace TVOS
{
	enum class Alignment
	{
		LeftTop = 0,
		LeftCenter = 1,
		LeftBottom = 2,
		CenterTop = 4,
		CenterCenter = 5,
		CenterBottom = 6,
		RightTop = 8,
		RightCenter = 9,
		RightBottom = 10,
	};

	bool IsLeft(Alignment alignment)
	{
		return int(alignment) & (int(Alignment::LeftTop) | int(Alignment::LeftCenter) | int(Alignment::LeftBottom)) ? true : false;
	}

	bool IsTop(Alignment alignment)
	{
		return int(alignment) & (int(Alignment::LeftTop) | int(Alignment::CenterTop) | int(Alignment::RightTop)) ? true : false;
	}

	bool IsRight(Alignment alignment)
	{
		return int(alignment) & (int(Alignment::RightTop) | int(Alignment::RightCenter) | int(Alignment::RightBottom)) ? true : false;
	}

	bool IsBottom(Alignment alignment)
	{
		return int(alignment) & (int(Alignment::LeftBottom) | int(Alignment::CenterBottom) | int(Alignment::RightBottom)) ? true : false;
	}

	bool IsCenter(Alignment alignment)
	{
		return int(alignment) & (int(Alignment::LeftCenter) | int(Alignment::CenterCenter) | int(Alignment::RightCenter)) ? true : false;
	}

	class UIElementBase
	{
	protected:
		Graphics& FB;
		std::string Name;

	public:
		UIElementBase(Graphics& FB, const std::string& Name);
		const std::string& GetName() const;

		int ArrangedRelX = 0;
		int ArrangedRelY = 0;
		int ArrangedWidth = 0;
		int ArrangedHeight = 0;
		int ArrangedAbsX = 0;
		int ArrangedAbsY = 0;

		int XPadding = 0;
		int YPadding = 0;
		int XBorder = 0;
		int YBorder = 0;
		int XMargin = 0;
		int YMargin = 0;
		bool Transparent = false;
		uint32_t FillColor = 0xFFFFFFFF;
		uint32_t BorderColor = 0;
		Alignment alignment = Alignment::CenterCenter;
		bool ExpandToParent = true;
		std::map<std::string, std::shared_ptr<UIElementBase>> SubElements;

		std::shared_ptr<UIElementBase> FindElement(const std::string& Name);

		int GetFrameWidth() const;
		int GetFrameHeight() const;

		void ArrangeElements(int WidthLimit, int HeightLimit, int& ActualWidth, int& TotalHeight);
	 
		virtual void Render(int x, int y, int w, int h);
	};

	class UIElementLabel : public UIElementBase
	{
	protected:
		std::string Caption;
		int CaptionWidth = 0;
		int CaptionHeight = 22;

	public:
		UIElementLabel(Graphics& FB, const std::string& Name);

		uint32_t FontColor = 0xFFFFFFFF;

		void SetCaption(const std::string& Caption);
		const std::string& GetCaption() const;
		void GetCaptionSize(int& w, int& h) const;

		virtual void Render(int x, int y, int w, int h) const;
	};






}


