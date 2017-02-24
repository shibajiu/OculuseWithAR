#pragma once
template <class T>
class KalmanFilter {
private:
	T cov_Q, cov_R;
	T cov_P_prior;
	T input_k;
	T val_x_prior_predict;
	bool isFirst = true;
public:
	KalmanFilter(T _q, T _r, T _p);
	~KalmanFilter();
	void SetInput(T _z);
	T Process(T _in);
};

template<class T>
KalmanFilter<T>::KalmanFilter(T _q, T _r, T _p) :cov_Q(_q), cov_R(_r), cov_P_prior(_p) {}

template<class T>
KalmanFilter<T>::~KalmanFilter() {}

template<class T>
void KalmanFilter<T>::SetInput(T _z) {
	z = _z;
}

template<class T>
T KalmanFilter<T>::Process(T _in) {
	if (isFirst) {
		val_x_prior_predict = _in;
		isFirst = false;
		return _in;
	}
	T _tx = val_x_prior_predict;
	T _tp = cov_P_prior + cov_Q;
	T _ty = _in - _tx;
	T _ts = _tp + cov_R;
	T _k = _tp / _ts;
	val_x_prior_predict = _tx + _k*_ty;
	cov_P_prior = _tp - _k*_tp;
	return val_x_prior_predict;
}