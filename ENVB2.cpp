/******************************* Preamble **************************
* code belonging to ENVB2.pdf					   *						
* version: 01							   *
* author: Magnus M�nch						   *
* created: 25-11-2016						   *
* last edited: 29-11-2016					   *
*******************************************************************/

/******************************* Notes *****************************
* 25-11-2016: gives negative variances                             *
*******************************************************************/

#include <math.h>
#include <RcppArmadillo.h>

// [[Rcpp::depends(RcppArmadillo)]]

using namespace Rcpp;
using namespace arma;

// [[Rcpp::export]]
List est_param(mat x, vec kappa, vec m, int n, int p, vec ciold, vec phi, vec chiold, vec lambda2, bool intercept) {
    vec evec(n,fill::ones);
    mat xaug = join_rows(evec,x);
    mat trx = x.t();
    vec h = lambda2 + lambda2%sqrt(phi/chiold);  
    vec om = 0.5*m/ciold%tanh(ciold/2);
    vec ominv = 1/om;
    vec hinv = 1/h;

    double invtrom;
    mat Omadj(n,n);
    mat txhinv = trx.each_col() % hinv;
    mat xtrxom(n,n);
    mat Ainv(p,p);
    vec ainvxom(p);
    mat sigma(p + 1,p + 1);
    vec sigmapart(p);
    vec mu(p + 1);
    vec varmean;
    
    if (intercept) {
      invtrom = 1/sum(om);
      Omadj = diagmat(om) - invtrom*(om*om.t());
      xtrxom = x*txhinv*Omadj;
      xtrxom.diag() += 1;
      Ainv= diagmat(hinv) - txhinv*Omadj*xtrxom.i()*txhinv.t();
      ainvxom = Ainv*trx*om;
      sigmapart = -invtrom*ainvxom;
      sigma(0,0) = as_scalar(invtrom + pow(invtrom,2.0)*(om.t()*x*ainvxom));
      sigma.submat(1,0,p,0) = sigmapart;
      sigma.submat(0,1,0,p) = sigmapart.t();
      sigma.submat(1,1,p,p) = Ainv;
      mu = sigma*xaug.t()*kappa;
      ciold = sqrt(diagvec(xaug*sigma*xaug.t()) + square(xaug*mu));
      varmean = diagvec(sigma) + square(mu);
      chiold = lambda2%varmean.rows(1,p);
    } else {
      xtrxom = x * txhinv + diagmat(ominv);
      sigma = diagmat(hinv) - txhinv*xtrxom.i()*txhinv.t();
      mu = sigma*trx*kappa;
      ciold = sqrt(diagvec(x*sigma*trx) + square(x*mu));
      varmean = diagvec(sigma) + square(mu);
      chiold = lambda2%varmean;
    }

    return List::create(Named("sigma") = sigma,
                        Named("mu") = mu,
                        Named("ci") = ciold,
                        Named("chi") = chiold);
}
