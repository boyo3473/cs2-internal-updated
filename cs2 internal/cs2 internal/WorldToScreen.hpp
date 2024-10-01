#pragma ones
struct view_matrix_t {
    float matrix[4][4];
};

Vector3 worldToScreen(const view_matrix_t& matrix, const Vector3& worldPos) {
    float _x = matrix.matrix[0][0] * worldPos.x + matrix.matrix[0][1] * worldPos.y + matrix.matrix[0][2] * worldPos.z + matrix.matrix[0][3];
    float _y = matrix.matrix[1][0] * worldPos.x + matrix.matrix[1][1] * worldPos.y + matrix.matrix[1][2] * worldPos.z + matrix.matrix[1][3];
    float _w = matrix.matrix[3][0] * worldPos.x + matrix.matrix[3][1] * worldPos.y + matrix.matrix[3][2] * worldPos.z + matrix.matrix[3][3];

    float inv_w = 1.f / _w;
    _x *= inv_w;
    _y *= inv_w;

    int screen_x = static_cast<int>((0.5f * _x + 0.5f) * static_cast<float>(GetSystemMetrics(SM_CXSCREEN)));
    int screen_y = static_cast<int>((-0.5f * _y + 0.5f) * static_cast<float>(GetSystemMetrics(SM_CYSCREEN)));

    return Vector3(static_cast<float>(screen_x), static_cast<float>(screen_y), _w);
}