#pragma once
#include "graphics.hpp"

#include <map>
#include <memory>
#include <string>

namespace TVOS
{
	enum class AlignmentType
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

	bool IsLeft(AlignmentType alignment);
	bool IsTop(AlignmentType alignment);
	bool IsRight(AlignmentType alignment);
	bool IsBottom(AlignmentType alignment);
	bool IsCenter(AlignmentType alignment);

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
		int ArrangedContentsWidth = 0;
		int ArrangedContentsHeight = 0;
		int ArrangedContainerWidth = 0;
		int ArrangedContainerHeight = 0;
		int ArrangedAbsX = 0;
		int ArrangedAbsY = 0;

		int XPadding = 0;
		int YPadding = 0;
		int XBorder = 0;
		int YBorder = 0;
		int XMargin = 0;
		int YMargin = 0;
		bool Transparent = false;
		bool ClipChildren = false;
		uint32_t FillColor = 0xFFFFFFFF;
		uint32_t BorderColor = 0;
		AlignmentType Alignment = AlignmentType::LeftTop;
		bool ExpandToParentX = false;
		bool ExpandToParentY = false;
		bool LineBreak = false;

		std::shared_ptr<UIElementBase> FindElement(const std::string& Name);

		int GetFrameWidth() const;
		int GetFrameHeight() const;

		// 使所有的子组件进行顺序位置的排列，然后统计出 Bounding Box 的大小。
		// 子组件的 `ArrangedRelX` `ArrangedRelY` `ArrangedWidth` `ArrangedHeight` `ArrangedContainerWidth` `ArrangedContainerHeight` 会被修改。
		virtual void GetClientContentsSize(int WidthLimit, int HeightLimit, int& ActualWidth, int& TotalHeight);

		void ArrangeSubElementsAbsPos(int x, int y);

		// 使自身和所有子组件进行顺序位置的排列。
		void ArrangeElements(int x, int y, int w, int h);
	 
		virtual void Render(int x, int y, int w, int h);
		void Render();

	protected:
		std::map<std::string, std::shared_ptr<UIElementBase>> SubElementsMap;
		std::vector<std::shared_ptr<UIElementBase>> SubElements;

	public:
		std::shared_ptr<UIElementBase> GetElementByName(const std::string& Name);

		UIElementBase& InsertElement(std::shared_ptr<UIElementBase> Element);
		bool RemoveElement(const std::string& Name);
		void ClearElements();

		decltype(SubElements.cbegin()) cbegin() const;
		decltype(SubElements.cend()) cend() const;
		decltype(SubElements.begin()) begin();
		decltype(SubElements.end()) end();
		decltype(SubElements.size()) size() const;
		decltype(SubElements.front()) front();
		decltype(SubElements.back()) back();
		decltype(SubElements.at(size_t(0))) at(size_t Index);
		decltype(SubElementsMap.at("")) at(const std::string& Name);
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

		virtual void GetClientContentsSize(int WidthLimit, int HeightLimit, int& ActualWidth, int& TotalHeight);

		void SetCaption(const std::string& Caption);
		const std::string& GetCaption() const;
		void GetCaptionSize(int& w, int& h) const;

		virtual void Render(int x, int y, int w, int h);
	};

	




}


