#include "LayoutEngine.hpp"

namespace behavior::hud {

std::vector<ElementLayout> LayoutEngine::calculate(Type type,
                                                    const std::vector<graphic::Vector2f>& sizes,
                                                    float padding,
                                                    graphic::Vector2f containerSize,
                                                    int groupStride)
{
    switch (type) {
        case Type::Horizontal:          return horizontal(sizes, padding, containerSize);
        case Type::MediaObject:         return mediaObject(sizes, padding, containerSize);
        case Type::VerticalMedia:       return verticalMedia(sizes, padding, containerSize, groupStride);
        case Type::Grid:                return grid(sizes, padding, containerSize, groupStride > 0 ? groupStride : 1);
        case Type::MediaObjectHButtons: return mediaObjectHButtons(sizes, padding, containerSize, groupStride);
        default:                        return vertical(sizes, padding, containerSize);
    }
}

std::vector<ElementLayout> LayoutEngine::vertical(const std::vector<graphic::Vector2f>& sizes,
                                                   float padding, graphic::Vector2f containerSize)
{
    std::vector<ElementLayout> out;
    out.reserve(sizes.size());
    float y = padding;
    for (auto& s : sizes) {
        float x = (containerSize.x - s.x) / 2.0f; // center horizontally
        out.push_back({{x, y}, s});
        y += s.y + padding;
    }
    return out;
}

std::vector<ElementLayout> LayoutEngine::horizontal(const std::vector<graphic::Vector2f>& sizes,
                                                     float padding, graphic::Vector2f containerSize)
{
    std::vector<ElementLayout> out;
    out.reserve(sizes.size());

    float totalW = 0.f;
    for (auto& s : sizes) totalW += s.x;
    totalW += padding * (static_cast<float>(sizes.size()) - 1.f);

    float x = (containerSize.x - totalW) / 2.0f;
    for (auto& s : sizes) {
        float y = (containerSize.y - s.y) / 2.0f; // center vertically
        out.push_back({{x, y}, s});
        x += s.x + padding;
    }
    return out;
}

std::vector<ElementLayout> LayoutEngine::mediaObject(const std::vector<graphic::Vector2f>& sizes,
                                                      float padding, graphic::Vector2f containerSize)
{
    if (sizes.empty()) return {};

    std::vector<ElementLayout> out;
    out.reserve(sizes.size());

    const graphic::Vector2f& mediaSz = sizes[0];

    // compute right-column total height
    float detailH = 0.0f;
    for (std::size_t i = 1; i < sizes.size(); ++i) {
        if (i > 1) detailH += padding;
        detailH += sizes[i].y;
    }

    float rightColX = padding + mediaSz.x + padding;
    float rightColW = containerSize.x - rightColX - padding;

    float mediaY = padding + std::max(0.0f, (detailH - mediaSz.y) * 0.5f);
    out.push_back({{padding, mediaY}, mediaSz});

    float detailStartY = padding + std::max(0.0f, (mediaSz.y - detailH) * 0.5f);
    float y = detailStartY;
    for (std::size_t i = 1; i < sizes.size(); ++i) {
        graphic::Vector2f sz = {std::min(sizes[i].x, rightColW), sizes[i].y};
        out.push_back({{rightColX, y}, sz});
        y += sz.y + padding;
    }

    return out;
}

std::vector<ElementLayout> LayoutEngine::verticalMedia(const std::vector<graphic::Vector2f>& sizes,
                                                        float padding, graphic::Vector2f containerSize,
                                                        int stride)
{
    if (sizes.empty()) return {};

    std::vector<ElementLayout> out(sizes.size());
    int n      = static_cast<int>(sizes.size());
    float y    = padding;
    int   i    = 0;

    while (i < n) {
        int end = (stride > 0) ? std::min(i + stride, n) : n;

        float topX = (containerSize.x - sizes[i].x) / 2.f;
        out[i]     = {{topX, y}, sizes[i]};
        float groupH = sizes[i].y;

        if (end - i > 1) {
            float totalW = 0.f;
            for (int j = i + 1; j < end; ++j) totalW += sizes[j].x;
            totalW += padding * static_cast<float>(end - i - 2);

            float x      = (containerSize.x - totalW) / 2.f;
            float btnY   = y + sizes[i].y + padding;
            float maxBtnH = 0.f;
            for (int j = i + 1; j < end; ++j) {
                out[j]   = {{x, btnY}, sizes[j]};
                x       += sizes[j].x + padding;
                maxBtnH  = std::max(maxBtnH, sizes[j].y);
            }
            groupH += padding + maxBtnH;
        }

        y += groupH + padding;
        i  = end;
    }

    return out;
}

std::vector<ElementLayout> LayoutEngine::grid(const std::vector<graphic::Vector2f>& sizes,
                                               float padding, graphic::Vector2f containerSize,
                                               int columns)
{
    if (sizes.empty() || columns <= 0) return {};

    std::vector<ElementLayout> out(sizes.size());
    int n = static_cast<int>(sizes.size());

    float cellW = (containerSize.x - padding * (static_cast<float>(columns) + 1.f)) / static_cast<float>(columns);

    float y = padding;
    int i = 0;
    while (i < n) {
        int rowEnd = std::min(i + columns, n);
        float rowH = 0.f;
        for (int j = i; j < rowEnd; ++j)
            rowH = std::max(rowH, sizes[j].y);

        for (int j = i; j < rowEnd; ++j) {
            int col = j - i;
            float x = padding + static_cast<float>(col) * (cellW + padding) + (cellW - sizes[j].x) * 0.5f;
            float ey = y + (rowH - sizes[j].y) * 0.5f;
            out[j] = {{x, ey}, sizes[j]};
        }

        y += rowH + padding;
        i = rowEnd;
    }

    return out;
}

// image [0] left column; [1..N-buttonCount] text stacked in right column;
// [N-buttonCount..N] buttons in a horizontal row at the bottom of the right column.
std::vector<ElementLayout> LayoutEngine::mediaObjectHButtons(
    const std::vector<graphic::Vector2f>& sizes,
    float padding, graphic::Vector2f containerSize, int buttonCount)
{
    if (sizes.empty()) return {};

    std::vector<ElementLayout> out;
    out.reserve(sizes.size());

    const graphic::Vector2f& mediaSz = sizes[0];
    int n = static_cast<int>(sizes.size());
    int btnStart = (buttonCount > 0) ? std::max(1, n - buttonCount) : n;

    float rightColX = padding + mediaSz.x + padding;
    float rightColW = containerSize.x - rightColX - padding;

    // --- button row height ---
    float btnRowH = 0.f;
    float btnTotalW = 0.f;
    for (int i = btnStart; i < n; ++i) {
        btnRowH = std::max(btnRowH, sizes[i].y);
        btnTotalW += sizes[i].x;
    }
    if (n - btnStart > 1)
        btnTotalW += padding * static_cast<float>(n - btnStart - 1);

    // --- text stack height ---
    float textH = 0.f;
    for (int i = 1; i < btnStart; ++i) {
        if (i > 1) textH += padding;
        textH += sizes[i].y;
    }

    float innerH = textH + (btnStart < n ? padding + btnRowH : 0.f);
    float mediaY = padding + std::max(0.f, (innerH - mediaSz.y) * 0.5f);
    out.push_back({{padding, mediaY}, mediaSz});

    float textStartY = padding + std::max(0.f, (mediaSz.y - innerH) * 0.5f);
    float y = textStartY;
    for (int i = 1; i < btnStart; ++i) {
        graphic::Vector2f sz = {std::min(sizes[i].x, rightColW), sizes[i].y};
        out.push_back({{rightColX, y}, sz});
        y += sz.y + padding;
    }

    // buttons: horizontal row, left-aligned in right column
    float bx = rightColX;
    for (int i = btnStart; i < n; ++i) {
        float bw = std::min(sizes[i].x, rightColW);
        out.push_back({{bx, y}, {bw, sizes[i].y}});
        bx += bw + padding;
    }

    return out;
}

} // namespace behavior::hud
