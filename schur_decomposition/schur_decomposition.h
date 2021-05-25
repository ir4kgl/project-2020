#include "../eigen/Eigen/Dense"
#include "../schur_decomposition/givens_rotation.h"
#include "../schur_decomposition/hessenberg_reduction.h"

namespace schur_decomposition {

template <typename Scalar>
class SchurDecomposition {
  static_assert(std::is_arithmetic_v<Scalar>,
                "Scalar must be arithmetic type!");

 public:
  using HessenbergReduction = hessenberg_reduction::HessenbergReduction<Scalar>;
  using Rotator = givens_rotation::GivensRotator<Scalar>;
  using Reflector = HessenbergReduction::HouseholderReflector;

  using DynamicMatrix = HessenbergReduction::DynamicMatrix;
  using DynamicBlock = Eigen::Block<DynamicMatrix>;
  using Matrix3 = Eigen::Matrix<Scalar, 3, 3>;
  using Vector3 = Eigen::Matrix<Scalar, 3, 1>;
  using Precision = Scalar;

  SchurDecomposition(Precision precision) : precision_(precision) {
    assert(precision >= 0);
  }

  void run(const DynamicMatrix& data, DynamicMatrix* schur_form,
           DynamicMatrix* unitary) {
    set_internal_members(data, schur_form, unitary);
    reduce_to_hessenberg_form();
    run_QR_algorithm();
  }

  void set_precision(Precision precision) {
    assert(precision >= 0);
    precision_ = precision;
  }

  Precision get_precision() const { return precision_; }

 private:
  void reduce_to_hessenberg_form() {
    HessenbergReduction reduction;
    reduction.run(p_schur_form_, p_unitary_);
  }

  void run_QR_algorithm() {
    for (cur_size_ = size() - 1; cur_size_ >= 2;) {
      start();
      process();
      finish();
      try_deflate();
    }
  }

  void start() {
    Reflector reflector = Reflector(find_starter_column());

    reflector.reflect_left(p_schur_form_->block(0, 0, 3, size()));
    reflector.reflect_right(
        p_schur_form_->block(0, 0, std::min(cur_size_, 3) + 1, 3));
    reflector.reflect_right(p_unitary_->block(0, 0, size(), 3));
  }

  void process() {
    for (int step = 0; step <= cur_size_ - 3; ++step) {
      Reflector reflector =
          Reflector(p_schur_form_->block(step + 1, step, 3, 1));

      reflector.reflect_left(
          p_schur_form_->block(step + 1, step, 3, size() - step));
      reflector.reflect_right(p_schur_form_->block(
          0, step + 1, std::min(cur_size_, step + 4) + 1, 3));
      reflector.reflect_right(p_unitary_->block(0, step + 1, size(), 3));
    }
  }

  void finish() {
    Reflector reflector =
        Reflector(p_schur_form_->block(cur_size_ - 1, cur_size_ - 2, 2, 1));

    reflector.reflect_left(p_schur_form_->block(cur_size_ - 1, cur_size_ - 2, 2,
                                                size() - cur_size_ + 2));
    reflector.reflect_right(
        p_schur_form_->block(0, cur_size_ - 1, cur_size_ + 1, 2));
    reflector.reflect_right(p_unitary_->block(0, cur_size_ - 1, size(), 2));
  }

  void try_deflate() {
    if (near_zero((*p_schur_form_)(cur_size_, cur_size_ - 1))) {
      deflate_once();
      return;
    }

    if (near_zero((*p_schur_form_)(cur_size_ - 1, cur_size_ - 2))) {
      deflate_twice();
    }
  }

  void deflate_once() {
    (*p_schur_form_)(cur_size_, cur_size_ - 1) = 0;
    cur_size_ -= 1;
  }

  void deflate_twice() {
    (*p_schur_form_)(cur_size_ - 1, cur_size_ - 2) = 0;
    cur_size_ -= 2;
  }

  Vector3 find_starter_column() {
    Scalar trace = find_bottom_corner_trace();
    Scalar det = find_bottom_corner_det();

    DynamicBlock top_corner = p_schur_form_->topLeftCorner(3, 3);
    Matrix3 starter_block = top_corner * top_corner - trace * top_corner +
                            det * Matrix3::Identity();
    return starter_block.col(0);
  }

  Scalar find_bottom_corner_trace() {
    return p_schur_form_->block(cur_size_ - 1, cur_size_ - 1, 2, 2).trace();
  }

  Scalar find_bottom_corner_det() {
    return p_schur_form_->block(cur_size_ - 1, cur_size_ - 1, 2, 2)
        .determinant();
  }

  bool near_zero(Scalar value) { return std::abs(value) < precision_; }

  void set_internal_members(const DynamicMatrix& data,
                            DynamicMatrix* schur_form, DynamicMatrix* unitary) {
    assert(data.rows() == data.cols());
    assert(schur_form);
    assert(unitary);

    p_schur_form_ = schur_form;
    p_unitary_ = unitary;
    *p_schur_form_ = data;
  }

  int size() {
    assert(p_schur_form_->rows() == p_schur_form_->cols());
    return p_schur_form_->rows();
  }

  Precision precision_;
  DynamicMatrix* p_schur_form_;
  DynamicMatrix* p_unitary_;
  int cur_size_;
};

};  // namespace schur_decomposition
