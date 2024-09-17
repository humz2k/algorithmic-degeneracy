#ifndef _CIRCULAR_BUFFER_HPP_
#define _CIRCULAR_BUFFER_HPP_

namespace deg {

template <typename T, int n> class circular_buffer {
  private:
    T m_buff[n] = {0};
    int m_count = 0;
    bool m_filled = false;

  public:
    void insert(T val) {
        m_buff[m_count] = val;
        m_count = (m_count + 1) % n;
        if (!m_filled && !m_count)
            m_filled = true;
    }

    T average() {
        const int n_eff = m_filled ? n : m_count;
        T sum = 0;
        for (int i = 0; i < n_eff; i++) {
            sum += m_buff[i];
        }
        return sum / (T)n_eff;
    }
};

} // namespace deg

#endif // _CIRCULAR_BUFFER_HPP_