#pragma once
#include <cstddef>

class Animation2D {
public:
    Animation2D(int numCols, int numRows, float frameTime = 0.1f)
        : m_numCols(numCols), m_numRows(numRows), m_frameTime(frameTime),
          m_currentCol(0), m_currentRow(0), m_timer(0.0f), m_colStart(0), m_colEnd(numCols-1) {}

    void SetColRange(int start, int end) {
        m_colStart = start;
        m_colEnd = end;
        if (m_currentCol < m_colStart || m_currentCol > m_colEnd) {
            m_currentCol = m_colStart;
        }
    }

    void Update(float deltaTime) {
        m_timer += deltaTime;
        if (m_timer >= m_frameTime) {
            m_timer -= m_frameTime;
            m_currentCol++;
            if (m_currentCol > m_colEnd) m_currentCol = m_colStart;
        }
    }

    void SetRow(int row) { m_currentRow = row; }
    void SetCol(int col) {
        if (col < m_colStart) col = m_colStart;
        if (col > m_colEnd) col = m_colEnd;
        m_currentCol = col;
    }
    int GetCol() const { return m_currentCol; }
    int GetRow() const { return m_currentRow; }
    int GetNumCols() const { return m_numCols; }
    int GetNumRows() const { return m_numRows; }

    // Tính UV cho frame hiện tại
    void GetUV(float& u0, float& v0, float& u1, float& v1) const {
        float du = 1.0f / m_numCols;
        float dv = 1.0f / m_numRows;
        u0 = m_currentCol * du;
        v0 = m_currentRow * dv;
        u1 = u0 + du;
        v1 = v0 + dv;
    }

private:
    int m_numCols, m_numRows;
    int m_currentCol, m_currentRow;
    float m_frameTime, m_timer;
    int m_colStart, m_colEnd;
}; 