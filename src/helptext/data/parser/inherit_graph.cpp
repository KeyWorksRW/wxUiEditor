/////////////////////////////////////////////////////////////////////////////
// Purpose:   Inheritance-graph SVG renderer (layered DAG layout).
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "inherit_graph.h"

#include <algorithm>
#include <cmath>
#include <format>
#include <limits>
#include <string_view>
#include <unordered_map>

namespace docparser
{

    // --- Layout constants (px; emitted 1:1 as SVG user units) ------------------
    constexpr double BOX_HEIGHT = 19.0;     // node box height
    constexpr double ROW_PITCH = 55.0;      // vertical centre-to-centre per rank
    constexpr double CHAR_WIDTH = 6.2;      // approx Helvetica 10px glyph advance
    constexpr double PAD_X = 7.0;           // horizontal padding inside a box
    constexpr double MIN_BOX_WIDTH = 40.0;  // minimum node box width
    constexpr double COL_GAP = 18.0;        // minimum gap between boxes in a row
    constexpr double MARGIN = 8.0;          // outer margin around the drawing
    constexpr double ARROW_LEN = 8.0;       // arrowhead length
    constexpr double ARROW_HALF_W = 3.5;    // arrowhead half-width

    // Doxygen-like palette.
    constexpr std::string_view COLOR_NODE_FILL = "#ffffff";
    constexpr std::string_view COLOR_NODE_HILITE = "#bfbfbf";
    constexpr std::string_view COLOR_NODE_STROKE = "#666666";
    constexpr std::string_view COLOR_EDGE = "#63b8ff";

    static std::string XmlEscape(std::string_view text)
    {
        std::string result;
        result.reserve(text.size());
        for (const char character: text)
        {
            switch (character)
            {
                case '&':
                    result += "&amp;";
                    break;
                case '<':
                    result += "&lt;";
                    break;
                case '>':
                    result += "&gt;";
                    break;
                case '"':
                    result += "&quot;";
                    break;
                case '\'':
                    result += "&#39;";
                    break;
                default:
                    result += character;
                    break;
            }
        }
        return result;
    }

    static std::string Num(double value)
    {
        // Snap tiny magnitudes to zero so "-0.00" never appears.
        if (std::abs(value) < 0.005)
        {
            return "0";
        }
        return std::format("{:.2f}", value);
    }

    static double BoxWidth(std::string_view name)
    {
        const double text_width = (static_cast<double>(name.size()) * CHAR_WIDTH) + (2.0 * PAD_X);
        return std::max(MIN_BOX_WIDTH, text_width);
    }

    std::string RenderInheritanceSvg(std::span<const InheritGraphNode> nodes, double* out_width,
                                     double* out_height, bool horizontal)
    {
        const std::size_t count = nodes.size();
        if (count == 0)
        {
            return {};
        }

        // -- Map names to indices and build parent/child adjacency ----------------
        std::unordered_map<std::string_view, int> index_of;
        index_of.reserve(count);
        for (std::size_t i = 0; i < count; ++i)
        {
            index_of.emplace(nodes[i].name, static_cast<int>(i));
        }

        std::vector<std::vector<int>> parents(count);
        std::vector<std::vector<int>> children(count);
        for (std::size_t i = 0; i < count; ++i)
        {
            for (const std::string& base: nodes[i].bases)
            {
                const std::unordered_map<std::string_view, int>::const_iterator found =
                    index_of.find(base);
                if (found != index_of.end() && found->second != static_cast<int>(i))
                {
                    parents[i].push_back(found->second);
                    children[found->second].push_back(static_cast<int>(i));
                }
            }
        }

        // -- Rank each node by longest path from a root (DAG relaxation) ----------
        std::vector<int> rank(count, 0);
        for (std::size_t pass = 0; pass < count; ++pass)
        {
            bool changed = false;
            for (std::size_t i = 0; i < count; ++i)
            {
                for (const int parent: parents[i])
                {
                    if (rank[parent] + 1 > rank[i])
                    {
                        rank[i] = rank[parent] + 1;
                        changed = true;
                    }
                }
            }
            if (!changed)
            {
                break;
            }
        }

        int max_rank = 0;
        for (const int value: rank)
        {
            max_rank = std::max(max_rank, value);
        }

        // -- Group node indices by rank (insertion order within a rank) -----------
        std::vector<std::vector<int>> rows(static_cast<std::size_t>(max_rank) + 1);
        for (std::size_t i = 0; i < count; ++i)
        {
            rows[static_cast<std::size_t>(rank[i])].push_back(static_cast<int>(i));
        }

        // -- Geometry -------------------------------------------------------------
        std::vector<double> width(count);
        std::vector<double> center_x(count, 0.0);
        std::vector<double> center_y(count, 0.0);

        if (!horizontal)
        {
            // --- Vertical layout (default) — ranks are rows, relaxation on X ---
            for (std::size_t i = 0; i < count; ++i)
            {
                width[i] = BoxWidth(nodes[i].name);
                center_y[i] = (static_cast<double>(rank[i]) * ROW_PITCH) + (BOX_HEIGHT / 2.0);
            }

            // Seed x: lay each row out left-to-right.
            for (const std::vector<int>& row: rows)
            {
                double cursor = 0.0;
                for (const int node: row)
                {
                    center_x[node] = cursor + (width[node] / 2.0);
                    cursor += width[node] + COL_GAP;
                }
            }
        }
        else
        {
            // --- Horizontal layout — ranks are columns, direct centering on Y ---
            //
            // Barycentre relaxation on Y doesn't work here: when every child's
            // desired Y is the parent's Y and the parent's desired Y is the average
            // of the children, the system drifts downward indefinitely. Instead,
            // position the rightmost (highest rank) column evenly, then center
            // each node at the average Y of its connected neighbours in higher
            // ranks, working right-to-left. This converges in one pass.

            for (std::size_t i = 0; i < count; ++i)
            {
                width[i] = BoxWidth(nodes[i].name);
            }

            // Compute column width per rank = widest box in that column.
            std::vector<double> col_width(rows.size());
            for (std::size_t rank_idx = 0; rank_idx < rows.size(); ++rank_idx)
            {
                double max_w = MIN_BOX_WIDTH;
                for (const int node: rows[rank_idx])
                {
                    max_w = std::max(max_w, width[static_cast<std::size_t>(node)]);
                }
                col_width[rank_idx] = max_w;
            }

            // Position columns left-to-right with COL_GAP between column edges.
            double cursor_x = 0.0;
            for (std::size_t rank_idx = 0; rank_idx < rows.size(); ++rank_idx)
            {
                const double col_centre = cursor_x + (col_width[rank_idx] / 2.0);
                for (const int node: rows[rank_idx])
                {
                    center_x[static_cast<std::size_t>(node)] = col_centre;
                }
                cursor_x += col_width[rank_idx] + COL_GAP;
            }

            // Position the rightmost column (highest rank) evenly spaced.
            // For typical viewer graphs this is the children column.
            {
                const std::size_t last_rank = rows.size() - 1;
                const std::vector<int>& last_col = rows[last_rank];
                double cursor_y = 0.0;
                for (const int node: last_col)
                {
                    const std::size_t node_idx = static_cast<std::size_t>(node);
                    center_y[node_idx] = cursor_y + (BOX_HEIGHT / 2.0);
                    cursor_y += BOX_HEIGHT + COL_GAP;
                }
            }

            // Now work right-to-left through remaining ranks, centering each node
            // on its children in already-positioned higher ranks.
            for (std::size_t rank_idx = rows.size(); rank_idx-- > 0;)
            {
                if (rank_idx == rows.size() - 1)
                {
                    continue;  // already positioned above
                }
                for (const int node: rows[rank_idx])
                {
                    const std::size_t node_idx = static_cast<std::size_t>(node);
                    double y_sum = 0.0;
                    int y_count = 0;
                    for (const int child: children[node_idx])
                    {
                        y_sum += center_y[static_cast<std::size_t>(child)];
                        ++y_count;
                    }
                    if (y_count > 0)
                    {
                        center_y[node_idx] = y_sum / y_count;
                    }
                }
            }
        }

        // -- Barycentre relaxation (vertical mode only) ----------------------------
        // For vertical layout: operates on center_x (horizontal position within row).
        if (!horizontal)
        {
            constexpr int TOTAL_PASSES = 14;
            constexpr int REORDER_PASSES = 5;
            std::vector<double> desired(count, 0.0);
            for (int pass = 0; pass < TOTAL_PASSES; ++pass)
            {
                for (std::size_t i = 0; i < count; ++i)
                {
                    double sum = 0.0;
                    int neighbors = 0;
                    for (const int parent: parents[i])
                    {
                        sum += center_x[static_cast<std::size_t>(parent)];
                        ++neighbors;
                    }
                    for (const int child: children[i])
                    {
                        sum += center_x[static_cast<std::size_t>(child)];
                        ++neighbors;
                    }
                    desired[i] = (neighbors > 0) ? (sum / neighbors) : center_x[i];
                }

                const bool may_reorder = (pass < REORDER_PASSES);
                for (std::vector<int>& row: rows)
                {
                    if (may_reorder)
                    {
                        std::ranges::stable_sort(row,
                                                 [&](int lhs, int rhs)
                                                 {
                                                     return desired[static_cast<std::size_t>(lhs)] <
                                                            desired[static_cast<std::size_t>(rhs)];
                                                 });
                    }

                    double prev_right = -std::numeric_limits<double>::infinity();
                    for (const int node: row)
                    {
                        const std::size_t node_idx = static_cast<std::size_t>(node);
                        const double half = width[node_idx] / 2.0;
                        const double min_centre = prev_right + COL_GAP + half;
                        const double placed = std::max(desired[node_idx], min_centre);
                        center_x[node_idx] = placed;
                        prev_right = placed + half;
                    }
                }
            }
        }

        // -- Compute bounds and shift into positive space -------------------------
        double min_x = std::numeric_limits<double>::infinity();
        double max_x = -std::numeric_limits<double>::infinity();
        double min_y = std::numeric_limits<double>::infinity();
        double max_y = -std::numeric_limits<double>::infinity();
        for (std::size_t i = 0; i < count; ++i)
        {
            min_x = std::min(min_x, center_x[i] - (width[i] / 2.0));
            max_x = std::max(max_x, center_x[i] + (width[i] / 2.0));
            min_y = std::min(min_y, center_y[i] - (BOX_HEIGHT / 2.0));
            max_y = std::max(max_y, center_y[i] + (BOX_HEIGHT / 2.0));
        }

        const double shift_x = MARGIN - min_x;
        const double shift_y = MARGIN - min_y;
        for (std::size_t i = 0; i < count; ++i)
        {
            center_x[i] += shift_x;
            center_y[i] += shift_y;
        }
        const double doc_width = (max_x - min_x) + (2.0 * MARGIN);
        const double doc_height = (max_y - min_y) + (2.0 * MARGIN);
        if (out_width != nullptr)
        {
            *out_width = doc_width;
        }
        if (out_height != nullptr)
        {
            *out_height = doc_height;
        }

        // -- Emit SVG -------------------------------------------------------------
        std::string svg;
        svg.reserve(512 + (count * 256));
        svg += R"(<?xml version="1.0" encoding="UTF-8" standalone="no"?>)";
        svg += '\n';
        svg += std::format(
            R"(<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" )"
            R"(width="{0}pt" height="{1}pt" viewBox="0 0 {0} {1}">)",
            Num(doc_width), Num(doc_height));
        svg += '\n';

        // Edges first so node boxes paint on top of arrow tails.
        // In vertical mode: edge connects child_top → parent_bottom (parent above).
        // In horizontal mode: edge connects child_left → parent_right (parent on left).
        for (std::size_t i = 0; i < count; ++i)
        {
            double start_x = 0.0;
            double start_y = 0.0;
            if (!horizontal)
            {
                start_x = center_x[i];
                start_y = center_y[i] - (BOX_HEIGHT / 2.0);  // child top
            }
            else
            {
                start_x = center_x[i] - (width[i] / 2.0);  // child left
                start_y = center_y[i];
            }

            for (const int parent: parents[i])
            {
                const std::size_t p_idx = static_cast<std::size_t>(parent);
                double end_x = 0.0;
                double end_y = 0.0;
                if (!horizontal)
                {
                    end_x = center_x[p_idx];
                    end_y = center_y[p_idx] + (BOX_HEIGHT / 2.0);  // parent bottom
                }
                else
                {
                    end_x = center_x[p_idx] + (width[p_idx] / 2.0);  // parent right
                    end_y = center_y[p_idx];
                }

                const double delta_x = end_x - start_x;
                const double delta_y = end_y - start_y;
                const double length = std::max(0.001, std::hypot(delta_x, delta_y));
                const double unit_x = delta_x / length;
                const double unit_y = delta_y / length;

                // Arrowhead sits at the base (parent) end, pointing into it.
                const double base_x = end_x - (unit_x * ARROW_LEN);
                const double base_y = end_y - (unit_y * ARROW_LEN);
                const double perp_x = -unit_y;
                const double perp_y = unit_x;

                svg +=
                    std::format(R"(<path fill="none" stroke="{}" d="M{},{}L{},{}"/>)", COLOR_EDGE,
                                Num(start_x), Num(start_y), Num(base_x), Num(base_y));
                svg += '\n';
                svg += std::format(
                    R"(<polygon fill="{0}" stroke="{0}" points="{1},{2} {3},{4} {5},{6}"/>)",
                    COLOR_EDGE, Num(end_x), Num(end_y), Num(base_x + (perp_x * ARROW_HALF_W)),
                    Num(base_y + (perp_y * ARROW_HALF_W)), Num(base_x - (perp_x * ARROW_HALF_W)),
                    Num(base_y - (perp_y * ARROW_HALF_W)));
                svg += '\n';
            }
        }

        // Nodes.
        for (std::size_t i = 0; i < count; ++i)
        {
            const InheritGraphNode& node = nodes[i];
            const double box_x = center_x[i] - (width[i] / 2.0);
            const double box_y = center_y[i] - (BOX_HEIGHT / 2.0);
            const std::string_view fill = node.highlight ? COLOR_NODE_HILITE : COLOR_NODE_FILL;

            const std::string rect = std::format(
                R"(<rect x="{}" y="{}" width="{}" height="{}" fill="{}" stroke="{}"/>)", Num(box_x),
                Num(box_y), Num(width[i]), Num(BOX_HEIGHT), fill, COLOR_NODE_STROKE);
            const std::string text = std::format(
                R"(<text x="{}" y="{}" text-anchor="middle" font-family="Helvetica,sans-serif" )"
                R"(font-size="10">{}</text>)",
                Num(center_x[i]), Num(center_y[i] + 3.5), XmlEscape(node.name));

            const bool linked = !node.url.empty();
            if (linked)
            {
                svg += std::format(R"(<a xlink:href="{}" target="_top")", XmlEscape(node.url));
                if (!node.tooltip.empty())
                {
                    svg += std::format(R"( xlink:title="{}")", XmlEscape(node.tooltip));
                }
                svg += '>';
                svg += rect;
                svg += text;
                svg += "</a>";
            }
            else
            {
                svg += "<g>";
                if (!node.tooltip.empty())
                {
                    svg += std::format("<title>{}</title>", XmlEscape(node.tooltip));
                }
                svg += rect;
                svg += text;
                svg += "</g>";
            }
            svg += '\n';
        }

        svg += "</svg>\n";
        return svg;
    }

}  // namespace docparser
