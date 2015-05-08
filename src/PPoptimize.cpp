#include <RcppArmadillo.h> 
// [[Rcpp::depends(RcppArmadillo)]]
using namespace arma; 
using namespace Rcpp;

List VecSort(NumericVector IDdata,IntegerVector Auxdata) {
   NumericVector sortX = clone(IDdata);
   IntegerVector sortY = clone(Auxdata);
   
   int n = sortX.size();
   
   for(int i=0; i<(n-1); i++){
      for(int j=(i+1); j<n; j++){
          if(sortX(j) < sortX(i)){
            double temp = sortX(i);
            sortX(i) = sortX(j);
            sortX(j)=temp;
            int tempA = sortY(i);
            sortY(i) = sortY(j);
            sortY(j) = tempA;
          }
      }
   }
   
   return List::create(_["sortID"]=sortX,_["sortAux"]=sortY);
}

NumericMatrix NormalizeD(NumericMatrix rawdata){

   int n=rawdata.nrow(),p=rawdata.ncol();
   
   NumericMatrix normdata(n,p);
   NumericVector vmean(p),vs(p);
   
   for(int k=0;k<p;k++){
      for(int i=0; i<n; i++){
        vmean(k) += rawdata(i,k);
        vs(k) += pow(rawdata(i,k),2);
     }
     vmean(k) /=n;
     vs(k) = pow((vs(k) - n*pow(vmean(k),2))/(n-1),0.5);
   }
   for(int k=0;k<p;k++){
      for(int i=0; i<n; i++){
        normdata(i,k) = (rawdata(i,k)-vmean(k))/vs(k);
     }
   }
   return normdata;
 
}


NumericMatrix NormalizeProj(NumericMatrix proj){

   int p=proj.nrow(),q=proj.ncol();

   NumericMatrix normproj(p,q);
   double ss=0;
   for(int i=0; i<p; i++){
       ss += proj(i,0)*proj(i,0);
   }
   for(int i=0; i<p; i++){
       normproj(i,0) = proj(i,0)/sqrt(ss);
   }  
   
   if(q>1){
      for(int j=1; j<q; j++){
        double temp1 = 0,temp2 = 0;
        for(int i=0; i<p; i++){
            temp1 += proj(i,j)*normproj(i,j-1);
            temp2 += normproj(i,j-1)*normproj(i,j-1);
        }
        for(int i=0; i<p; i++){
           normproj(i,j) = proj(i,j)-normproj(i,j-1)*temp1/temp2; 
        }
      }
   }
   return normproj;
 
}

//' LDA PP index
//' 
//' Calculate LDA projection pursuit index value
//' @title LDA PPindex
//' @usage LDAindex(origclass,origdata,proj,weight=TRUE)
//' @param origclass class information vector
//' @param origdata data matrix without class information  
//' @param proj projection matrix 
//' @param weight weight flag in LDA
//' @references Lee, EK., Cook, D., Klinke, S., and Lumley, T.(2005) 
//' Projection Pursuit for exploratory supervised classification, 
//' Journal of Computational and Graphical statistics, 14(4):831-846.
//' @export
//' @keywords projection pursuit
//' @examples
//' data(iris)
//' LDAindex(iris[,5],as.matrix(iris[,1:4]))
//' @useDynLib PPtreeViz
//' @importFrom Rcpp evalCpp
// [[Rcpp::export]]

double LDAindex(IntegerVector origclass, NumericMatrix origdata, 
        NumericMatrix proj=NumericMatrix(0),bool weight=true){

   double index;
   int n=origdata.nrow(),p=origdata.ncol(),q=proj.ncol(),p1=proj.nrow();
   Environment base("package:base");
   Function table=base["table"];
   NumericVector gn=table(origclass);
   int g=gn.size();  
   if(p1!=p){ q = p;}
   NumericVector allmean(q);
   NumericMatrix W(q,q),WB(q,q),gsum(q,g);        
   NumericMatrix projdata(n,q);
   if(p1!=p || p1 ==1){
      projdata = origdata; 
   } else{
      for(int i=0; i<n; i++){
         for(int j=0; j<q; j++){
            for(int k=0; k<p; k++){
                 projdata(i,j) += origdata(i,k)*proj(k,j);
            }
         }
      }
   }
   for(int i=0;i<n;i++){
      for(int k=0;k<q;k++){
         allmean(k) += projdata(i,k)/n;
         gsum(k,(origclass(i)-1)) += projdata(i,k);
      }
   }

   for(int i=0; i<n; i++){
      int l = origclass[i]-1;
      double gn1;
      if(weight){
         gn1 = gn(l);
      } else{
         gn1 = n/g; 
      }
      for(int j1=0;j1<q;j1++){
         for(int j2=0;j2<=j1;j2++) {
            W(j1,j2) += ((projdata(i,j1)-gsum(j1,l)/gn(l))*
                         (projdata(i,j2)-gsum(j2,l)/gn(l)))/gn(l)*gn1;
            W(j2,j1) = W(j1,j2);
            double temp = ((projdata(i,j1)-gsum(j1,l)/gn(l))*
                           (projdata(i,j2)-gsum(j2,l)/gn(l))+
                              (gsum(j1,l)/gn(l)-allmean(j1))*
                              (gsum(j2,l)/gn(l)-allmean(j2)))/gn(l)*gn1;
            WB(j1,j2) += temp;
            WB(j2,j1) = WB(j1,j2);
         }
      }
   }
   Function det = base["det"];
   index =1.0-as<double>(det(wrap(W)))/as<double>(det(wrap(WB)));
   return index;
}
//' PDA PP index 
//' 
//' Calculate PDA projection pursuit index value 
//' @usage PDAindex(origclass,origdata,proj,weight=TRUE,lambda=0.1)
//' @param origclass class information vector
//' @param origdata data matrix without class information  
//' @param proj projection matrix 
//' @param weight weight flag in PDA
//' @param lambda lambda in PDA index
//' @references Lee, EK., Cook, D.(2010) 
//' A projection pursuit index for large p small n data, 
//' Statistics and Computing, 20:381-392.
//' @export
//' @keywords projection pursuit
//' @examples
//' data(iris)
//' PDAindex(iris[,5],as.matrix(iris[,1:4]),lambda=0.2)
//' 
//' 
// [[Rcpp::export]]
double PDAindex(IntegerVector origclass, NumericMatrix origdata,
        NumericMatrix proj = NumericMatrix(0) ,bool weight=true,double lambda=0.1){

   double index;
   int n=origdata.nrow(),p=origdata.ncol(),q=proj.ncol(),p1=proj.nrow();
   Environment base("package:base");
   Function table=base["table"];
   NumericVector gn=table(origclass);
   int g=gn.size();  
   NumericMatrix W(p,p),WB(p,p),gsum(p,g);
   NumericVector allmean(p);
   
     if(p1!=p){ q = p;}
   for(int i=0;i<n;i++){
      for(int k=0;k<p;k++){
         allmean(k) += origdata(i,k)/n;
         gsum(k,(origclass(i)-1)) += origdata(i,k);
      }
   }


   for(int i=0; i<n; i++){
      int l = origclass[i]-1;
      double gn1;
      if(weight){
         gn1 = gn(l);
      } else{
         gn1 = n/g; 
      }
      for(int j1=0;j1<p;j1++){
         for(int j2=0; j2<=j1; j2++) {
           double temp1,temp2;
            if(j1!=j2){
              temp1 =(1-lambda)* ((origdata(i,j1)-gsum(j1,l)/gn(l))*
                         (origdata(i,j2)-gsum(j2,l)/gn(l)))/gn(l)*gn1;

              temp2 = (1-lambda)*((origdata(i,j1)-gsum(j1,l)/gn(l))*
                           (origdata(i,j2)-gsum(j2,l)/gn(l)))+
                              (gsum(j1,l)/gn(l)-allmean(j1))*
                              (gsum(j2,l)/gn(l)-allmean(j2))/gn(l)*gn1;
            } else{
              temp1 = ((origdata(i,j1)-gsum(j1,l)/gn(l))*
                         (origdata(i,j2)-gsum(j2,l)/gn(l)))/gn(l)*gn1;

              temp2 = ((origdata(i,j1)-gsum(j1,l)/gn(l))*
                           (origdata(i,j2)-gsum(j2,l)/gn(l))+
                              (gsum(j1,l)/gn(l)-allmean(j1))*
                              (gsum(j2,l)/gn(l)-allmean(j2)))/gn(l)*gn1;              
            }  
            W(j1,j2) += temp1;  
            WB(j1,j2) += temp2;
            W(j2,j1) = W(j1,j2);            
            WB(j2,j1) = WB(j1,j2);
         }
      }      
   }

   NumericMatrix Wt(q,p),WBt(q,p);    
   NumericMatrix Wtt(q,q),WBtt(q,q); 
   if(p1!=p || p1 ==1){
      Wtt=W;
      WBtt=WB;
   } else{

      for(int i=0;i<p;i++){
         for(int j=0; j<q; j++){
            for(int k=0;k<p;k++){
               Wt(j,i) +=W(k,i)*proj(k,j);
               WBt(j,i) +=WB(k,i)*proj(k,j);               
            }
         }
      }
    
      for(int i=0;i<q;i++){
         for(int j=0; j<q; j++){
            for(int k=0;k<p;k++){
               Wtt(i,j) +=Wt(i,k)*proj(k,j);
               WBtt(i,j) +=WBt(i,k)*proj(k,j);               
            }
         }
      }      
   }
   
   Function det = base["det"];
   index =1.0-as<double>(det(wrap(Wtt)))/as<double>(det(wrap(WBtt)));
    
   return index;
}
//' Lr PP index
//' 
//' Calculate Lr projection pursuit index value
//' @usage Lrindex(origclass,origdata,proj,weight=TRUE,r=1)
//' @param origclass class information vector
//' @param origdata data matrix without class information  
//' @param proj projection matrix 
//' @param weight weight flag in Lr index
//' @param r r in Lr index
//' @references Lee, EK., Cook, D., Klinke, S., and Lumley, T.(2005) 
//' Projection Pursuit for exploratory supervised classification, 
//' Journal of Computational and Graphical statistics, 14(4):831-846.
//' @export
//' @keywords projection pursuit
//' @examples
//' data(iris)
//' Lrindex(iris[,5],as.matrix(iris[,1:4]),r=1)
//' 
// [[Rcpp::export]]

double Lrindex(IntegerVector origclass, NumericMatrix origdata,
NumericMatrix proj=NumericMatrix(0),bool weight=true,int r=1){
 
   double index,B=0,W=0;
   int n=origdata.nrow(),p=origdata.ncol(),q=proj.ncol(),p1=proj.nrow();
   Environment base("package:base");
   Function table = base["table"];
   NumericVector gn=table(origclass);
   int g=gn.size();   

   if(p1!=p){ q = p;}
   NumericVector allmean(q);
   NumericMatrix gsum(q,g);
   NumericMatrix projdata(n,q);
   if(p1!=p || p1 ==1){
      projdata = origdata; 
   } else{
      for(int i=0; i<n; i++){
         for(int j=0; j<q; j++){
            for(int k=0; k<p; k++){
                 projdata(i,j) += origdata(i,k)*proj(k,j);
            }
         }
      }
   }

   for(int i=0; i<n; i++){
      for(int k=0;k<q;k++){
         allmean(k) = allmean(k)+ projdata(i,k)/n;
         gsum(k,(origclass(i)-1)) += projdata(i,k);
      }
   }
   for(int i=0; i<n; i++){
      int l = origclass(i)-1;
      double gn1;
      if(weight){
         gn1 = gn(l);
      } else{
         gn1 = n/g; 
      }         
      for(int j=0;j<q;j++){
         W = W+ pow(pow((projdata(i,j)-gsum(j,l)/gn(l)),2*r),0.5)/gn(l)*gn1;
         B = B+ pow(pow((gsum(j,l)/gn(l)-allmean(j)),2*r),0.5)/gn(l)*gn1;
      }
   }
   
   W = pow(W,1.0/r); 
   double WB = pow(W+B,1.0/r);
   index = 1-W/WB;
   return index;
}

//' GINI PP index
//' 
//' Calculate GINI projection pursuit index value
//' @usage GINIindex1D(origclass,origdata,proj)
//' @param origclass class information vector
//' @param origdata data matrix without class information  
//' @param proj projection matrix
//' @export
//' @keywords projection pursuit
//' @examples
//' data(iris)
//' GINIindex1D(iris[,5],as.matrix(iris[,1,drop=FALSE]))
// [[Rcpp::export]]

double GINIindex1D(IntegerVector origclass, NumericMatrix origdata,
NumericVector proj=NumericVector(0)){
 
   Environment base("package:base");
   Function table = base["table"];
   NumericVector gn=table(origclass);
   int g=gn.size();
   int n=origdata.nrow(),p=origdata.ncol(),p1=proj.size();
   double n1,n2;
   double index=0,tempindex;
   NumericVector projdata(n);

   if(p1!=p || p1 ==1){
      projdata = origdata(_,0);
   } else{
      for(int i=0; i<n; i++){
         for(int k=0; k<p; k++){
              projdata(i) += origdata(i,k)*proj(k);
         }
      }
   } 

   List VecSortdata =  VecSort(projdata,origclass);
   NumericVector sortdata = as<NumericVector>(VecSortdata["sortID"]);
   IntegerVector sortclass = as<IntegerVector>(VecSortdata["sortAux"]);
   IntegerVector part1,part2,temptable1,temptable2;

   for(int i=1; i<(n-1); i++){  
      part1 = sortclass[sortdata<=sortdata[i]];
      part2 = sortclass[sortdata>sortdata[i]];
      n1 = part1.size();
      n2 = part2.size();
      temptable1 = table(part1); int g1 = temptable1.size();  tempindex=0;
      temptable2 = table(part2); int g2 = temptable2.size(); 
      for(int j=0; j<g1; j++){
         tempindex += ((n1)/n)*(temptable1(j)/n1)*(1.0-temptable1(j)/n1);
      }
      for(int j=0; j<g2; j++){         
         tempindex += ((n2)/n)*(temptable2(j)/n2)*(1.0-temptable2(j)/n2);        
      } 
      tempindex = g-1.0-g*tempindex;
      if(tempindex > index) 
         index = tempindex;             
   }  

   return index;
}

//' ENTROPY PP index 
//' 
//' Calculate ENTROPY projection pursuit index value
//' @usage ENTROPYindex1D(origclass,origdata,proj)
//' @param origclass class information vector
//' @param origdata data matrix without class information  
//' @param proj projection matrix
//' @export
//' @keywords projection pursuit
//' @examples
//' data(iris)
//' ENTROPYindex1D(iris[,5],as.matrix(iris[,1,drop=FALSE]))
// [[Rcpp::export]]


double ENTROPYindex1D(IntegerVector origclass, NumericMatrix origdata,
NumericVector proj=NumericVector(0)){
 
   Environment base("package:base");
   Function table = base["table"];
   NumericVector gn=table(origclass);
   int g=gn.size();
   int n=origdata.nrow(),p=origdata.ncol(),p1=proj.size();
   double n1,n2;
   double index=0,tempindex;
   NumericVector projdata(n);

   if(p1!=p || p1 ==1){
      projdata = origdata(_,0);
   } else{
      for(int i=0; i<n; i++){
         for(int k=0; k<p; k++){
              projdata(i) += origdata(i,k)*proj(k);
         }
      }
   } 

   List VecSortdata =  VecSort(projdata,origclass);
   NumericVector sortdata = as<NumericVector>(VecSortdata["sortID"]);
   IntegerVector sortclass = as<IntegerVector>(VecSortdata["sortAux"]);
   IntegerVector part1,part2,temptable1,temptable2;
 
   for(int i=1; i<(n-1); i++){  
      part1 = sortclass[sortdata<=sortdata[i]];
      part2 = sortclass[sortdata>sortdata[i]];
      n1 = part1.size();
      n2 = part2.size();
      temptable1 = table(part1); int g1 = temptable1.size();  tempindex=0;
      temptable2 = table(part2); int g2 = temptable2.size(); 
      for(int j=0; j<g1; j++){
         if(temptable1(j)!=0)
         {  tempindex += ((n1)/n)*(temptable1(j)/n1)*log(temptable1(j)/n1);
         }
      }
      for(int j=0; j<g2; j++){         
         if(temptable2(j)!=0)
         {  tempindex += ((n2)/n)*(temptable2(j)/n2)*log(temptable2(j)/n2);        
         }
      } 
      double maxI = log(2)-log(g);
      if((g/2)*2 != g){
        maxI = -0.5*log((g*g-1.0)/4.0)+1.0/(2.0*g)*log((g-1.0)/(g+1.0));
      }
      
      tempindex = (1+tempindex/log(g))/(1+maxI/log(g));
      if (tempindex > index) index = tempindex;            
   }   

   return index;
}

//' PP optimization using various PP indices
//' 
//' Find the q-dim optimal projection using various projectin pursuit indices with class information
//' @usage PPopt(origclass,origdata,q=1,PPmethod="LDA",weight=TRUE,r=1,lambda=0.1,
//'              energy=0,cooling=0.999,TOL=0.0001,maxiter = 50000)
//' @param origclass class information vector
//' @param origdata data matrix without class information
//' @param q dimension of projection matrix
//' @param PPmethod method for projection pursuit; "LDA", "PDA", "Lr", "GINI", and "ENTROPY"
//' @param weight weight flag in LDA, PDA and Lr index
//' @param r r in Lr index
//' @param lambda lambda in PDA index
//' @param energy energy parameter
//' @param cooling cooling parameter
//' @param TOL tolerance
//' @param maxiter number of maximum iteration
//' @return indexbest maximum LDA index value
//' @return projbest optimal q-dim projection matrix
//' @return origclass original class information vector
//' @return origdata  original data matrix  without class information
//' @references Lee, EK., Cook, D., Klinke, S., and Lumley, T.(2005) 
//' Projection Pursuit for exploratory supervised classification, 
//' Journal of Computational and Graphical statistics, 14(4):831-846.
//' @export
//' @keywords projection pursuit
//' @examples
//' data(iris)
//' PP.proj.result <- PPopt(iris[,5],as.matrix(iris[,1:4]))
//' PP.proj.result
// [[Rcpp::export]]
List PPopt(IntegerVector origclass, NumericMatrix origdata,int q=1, std::string PPmethod="LDA", 
           bool weight = true, int r=1,double lambda=0.1, 
           double energy=0, double cooling=0.999, double TOL=0.0001, int maxiter=50000){

   int n=origdata.nrow(),p=origdata.ncol();
   Environment base("package:base");
   Function table = base["table"];
   GetRNGstate();
   NumericMatrix projbest(p,q);
   double indexbest=0,newindex=0;
   if((PPmethod=="GINI" || PPmethod=="ENTROPY") && q>1)
   {  
      return Rcpp::List::create(Rcpp::Named("indexbest") = 0,
                             Rcpp::Named("projbest") = 0);
   } else {  
      if(PPmethod=="LDA"){
         indexbest = LDAindex(origclass,origdata,NumericMatrix(0),weight);
      } else if(PPmethod=="Lr"){
         indexbest = Lrindex(origclass,origdata,NumericMatrix(0),weight,r);
      } else if(PPmethod=="PDA"){
         indexbest = PDAindex(origclass,origdata,NumericMatrix(0),weight,lambda);  
      } else if(PPmethod=="GINI"){
         double tempindex=0;
         for(int i=0; i<p; i++){
            NumericVector tempproj(p);
            tempproj(i) = 1;
            tempindex = GINIindex1D(origclass,origdata,tempproj);  
            if(indexbest<tempindex)
                indexbest = tempindex;
         }        
      } else if(PPmethod=="ENTROPY"){
         double tempindex=0;
         for(int i=0; i<p; i++){
            NumericVector tempproj(p);
            tempproj(i) = 1;
            tempindex = ENTROPYindex1D(origclass,origdata,tempproj);  
            if(indexbest<tempindex)
                indexbest = tempindex;
         } 
      }   
   
      if(energy==0)
         energy = 1-indexbest;
   
      for(int k=0; k<q; k++){
         projbest(_,k)= rnorm(p);
      }
      projbest = NormalizeProj(projbest);  
      NumericMatrix projdata(n,q);  
  
      for(int i=0; i<n; i++){
         for(int k=0; k<q; k++){
            projdata(i,k)=0;
            for(int j=0; j<p; j++){
               projdata(i,k) += origdata(i,j)*projbest(j,k);
            }
         }
      }
  
      if(PPmethod=="LDA"){
         indexbest = LDAindex(origclass,origdata,projbest,weight);
      } else if(PPmethod=="Lr"){
         indexbest = Lrindex(origclass,origdata,projbest,weight,r);
      } else if(PPmethod=="PDA"){
         indexbest = PDAindex(origclass,origdata,projbest,weight,lambda);
      } else if(PPmethod=="GINI"){
         /*NumericMatrix tempdata;
         tempdata = projdata(_,0);*/
         indexbest = GINIindex1D(origclass,origdata,projbest);        
      } else if(PPmethod=="ENTROPY"){
         NumericVector tempdata;
         tempdata = projdata(_,0);
         indexbest = ENTROPYindex1D(origclass,origdata,projbest);        
      }
   
      double temp=1;
      int kk=0;
   
      double diff = 100;
      while(fabs(diff)>TOL && kk < maxiter){
         double tempp = energy/log(kk+2);
         if(kk>1000) {
            temp = temp*cooling;
         } else {
            temp = temp*cooling*cooling;
         }   
      
         NumericMatrix projnew(p,q);
         for(int k=0; k<q; k++){
            projnew(_,k)= temp*rnorm(p)+projbest(_,k);
         }
         projnew = NormalizeProj(projnew);  
        
         for(int i=0; i<n; i++){
            for(int k=0; k<q; k++){
               projdata(i,k)=0;
               for(int j=0; j<p; j++){
                  projdata(i,k) += origdata(i,j)*projnew(j,k);
               }
            }
         }
      
         if(PPmethod=="LDA"){
            newindex = LDAindex(origclass,origdata,projnew,weight);
         } else if(PPmethod=="Lr"){
            newindex = Lrindex(origclass,origdata,projnew,weight,r);
         } else if(PPmethod=="PDA"){
            newindex = PDAindex(origclass,origdata,projnew,weight,lambda);
         } else if(PPmethod=="GINI"){
            NumericVector tempproj= projnew(_,0);
            newindex = GINIindex1D(origclass,origdata,tempproj);        
         } else if(PPmethod=="ENTROPY"){
            NumericVector tempproj= projnew(_,0);
            newindex = ENTROPYindex1D(origclass,origdata,tempproj);        
         }
      
         NumericVector prob = runif(1);
         double difft = newindex - indexbest;
         double e = exp(difft/tempp);
         if(e>1){
            for(int i=0; i<p; i++){
               for(int j=0; j<q; j++){
                  projbest(i,j) = projnew(i,j);
               }
            }
            indexbest = newindex;    
            diff = difft;
         } else if(prob[0] < e && difft>energy){
            for(int i=0; i<p; i++){
               for(int j=0; j<q; j++){
                  projbest(i,j) = projnew(i,j);
               }
            }
            indexbest = newindex;    
            diff = difft; 
         }
         kk++;
      }
      PutRNGstate();
      return Rcpp::List::create(Rcpp::Named("indexbest") = indexbest,
                             Rcpp::Named("projbest") = projbest,
                             Rcpp::Named("origclass") = origclass,
                             Rcpp::Named("origdata") = origdata);
   }                           
}

