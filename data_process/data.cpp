#include <iostream>
#include <limits>
#include <stdlib.h>
#include <vector>

#include<cmath>
class AudioData{
public:
    int data_length;
    int dim;
    double* circle_data;
    int circle_point=0;
    double* right_data;
    double* compare_data;
    int compare_data_length;
    double* distance_data;

    AudioData(int data_length,int dim,double* compare_data,int compare_data_length){
        this->data_length=data_length;
        this->compare_data_length=compare_data_length;
        this->dim=dim;
        this->circle_data=(double *)malloc(sizeof(double)*data_length*dim);
        this->right_data= (double *)malloc(sizeof(double)*data_length*dim);
        this->compare_data=(double *)malloc(sizeof(double)*compare_data_length*dim);//compare_data_length*dim
        for(int i=0;i<compare_data_length*dim;i++){
            this->compare_data[i]=compare_data[i];
        }
        this->distance_data=(double *)malloc(sizeof(double)*compare_data_length*data_length);//data_length*compare_data_length
    }
    void put_number(double* number){
        if(this->circle_point==this->data_length){
            this->circle_point=0;
        }
        for(int i=0;i<dim;i++){
        this->circle_data[this->circle_point*dim+i]=number[i];
        }
        double dis=0;
        for(int i=0;i<this->compare_data_length;i++){
            dis=0;
            for(int j=0;j<dim;j++){
                dis+=std::abs(number[j]-compare_data[i*dim+j]);
            }
            distance_data[circle_point*compare_data_length+i]=dis;
        }

        
        this->circle_point += 1;


    }

    void put_number(std::vector<double> number){
        if(this->circle_point==this->data_length){
            this->circle_point=0;
        }
        for(int i=0;i<dim;i++){
        this->circle_data[this->circle_point*dim+i]=number[i];
        }
        double dis=0;
        for(int i=0;i<this->compare_data_length;i++){
            dis=0;
            for(int j=0;j<dim;j++){
                dis+=std::abs(number[j]-compare_data[i*dim+j]);
            }
            distance_data[circle_point*compare_data_length+i]=dis;
        }

        
        this->circle_point += 1;


    }

    void get_data(){
        int  correct_pointer=circle_point;
        for(int i=0;i<this->data_length;i++){
           if(correct_pointer>=this->data_length){
               correct_pointer-=this->data_length;
           }
           for(int j=0;j<dim;j++){
               this->right_data[i*dim+j]=this->circle_data[correct_pointer*dim+j];
           }
           
           correct_pointer +=1;
        }
    }

    double dtw(){

        float* sum_distance=new float[(this->data_length)*(this->compare_data_length)];
        sum_distance[0]=distance_data[0];
        int correct_pointer=circle_point;
        for(int i=1;i<this->data_length;i++){
            correct_pointer=circle_point+i;
             if(correct_pointer>=data_length){
                correct_pointer-=data_length;
            }
            sum_distance[i*(this->compare_data_length)]=sum_distance[(i-1)*(this->compare_data_length)]+distance_data[correct_pointer*(this->compare_data_length)];
        }
        correct_pointer=circle_point;
        for(int i=1;i<this->compare_data_length;i++){
             if(correct_pointer>=data_length){
                correct_pointer-=data_length;
            }
            sum_distance[i]=sum_distance[(i-1)]+distance_data[correct_pointer*(this->compare_data_length)+i];
        }
        
        double min_score=0;
        correct_pointer=circle_point;
        for(int i=1;i<data_length;i++){
            correct_pointer=circle_point+i;
            if(correct_pointer>=data_length){
                correct_pointer-=data_length;
            }
            for(int j=1;j<compare_data_length;j++){
                min_score=sum_distance[(i-1)*(this->compare_data_length)+j];
                if(min_score>sum_distance[(i-1)*(this->compare_data_length)+j-1]){
                    min_score=sum_distance[(i-1)*(this->compare_data_length)+j-1];
                }
                if(min_score>sum_distance[i*(this->compare_data_length)+j-1]){
                    min_score=sum_distance[i*(this->compare_data_length)+j-1];
                }
                sum_distance[i*(compare_data_length)+j]=min_score+distance_data[correct_pointer*(this->compare_data_length)+j];
                
            }
            
        }
    return sum_distance[(data_length-1)*(compare_data_length)+compare_data_length-1];

    }
    
};