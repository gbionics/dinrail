// SPDX-FileCopyrightText: Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_EIGEN_HELPER_H
#define DINRAIL_EIGEN_HELPER_H

#include <type_traits>

#include <Eigen/Core>

#include <dinrail/VectorProxy.h>

namespace dinrail {

/**
 * Utility function to create a mutable Eigen map from a reference to another
 * vector-like container.
 */
template <typename Class> auto to_eigen(Class &input) {
  static_assert(is_vector_constructible<Class>::value,
                "Cannot create a Vector from the input class. Cannot know how "
                "to convert to Eigen.");

  using value_type = typename container_data<Class>::type;
  auto genericVector = make_vector(input);
  return Eigen::Map<Eigen::Matrix<value_type, Eigen::Dynamic, 1>>(
      genericVector.data(), genericVector.size());
}

/**
 * Utility function to create a const Eigen map from a reference to another
 * vector-like container.
 */
template <typename Class> auto to_eigen(const Class &input) {
  static_assert(is_vector_constructible<Class>::value,
                "Cannot create a Vector from the input class. Cannot know how "
                "to convert to Eigen.");

  using value_type = std::remove_const_t<typename container_data<Class>::type>;
  auto genericVector = make_vector(input);
  return Eigen::Map<const Eigen::Matrix<value_type, Eigen::Dynamic, 1>>(
      genericVector.data(), genericVector.size());
}

} // namespace dinrail

#endif // DINRAIL_EIGEN_HELPER_H
