// Generated fixed C++ equations from the unchanged Python reference.
double classic(const std::string& calc_id,const std::vector<double>& values){
  if(((calc_id=="T_FROM_F"))){
    return (1.0/need_nonzero(values[0],"f"));
  }else{
    if(((calc_id=="F_FROM_T"))){
      return (1.0/need_nonzero(values[0],"T"));
    }else{
      if(((calc_id=="OMEGA_FROM_F"))){
        return ((2.0*PI)*values[0]);
      }else{
        if(((calc_id=="F_FROM_OMEGA"))){
          return (values[0]/(2.0*PI));
        }else{
          if(((calc_id=="T_FROM_OMEGA"))){
            return ((2.0*PI)/need_nonzero(values[0],"omega"));
          }else{
            if(((calc_id=="V_FROM_S_T"))){
              return (values[0]/need_nonzero(values[1],"t"));
            }else{
              if(((calc_id=="S_FROM_V_T"))){
                return (values[0]*values[1]);
              }else{
                if(((calc_id=="A_FROM_DV_DT"))){
                  return (values[0]/need_nonzero(values[1],"dt"));
                }else{
                  if(((calc_id=="F_FROM_M_A"))){
                    return (values[0]*values[1]);
                  }else{
                    if(((calc_id=="P_FROM_M_V"))){
                      return (values[0]*values[1]);
                    }else{
                      if(((calc_id=="E_KIN"))){
                        return (((0.5*values[0])*values[1])*values[1]);
                      }else{
                        if(((calc_id=="E_POT"))){
                          return ((values[0]*values[1])*values[2]);
                        }else{
                          if(((calc_id=="P_MECH"))){
                            return (values[0]/need_nonzero(values[1],"t"));
                          }else{
                            if(((calc_id=="T_SPRING"))){
                              return ((2.0*PI)*need_sqrt((values[0]/need_nonzero(values[1],"D"))));
                            }else{
                              if(((calc_id=="D_SPRING"))){
                                return ((((4.0*PI)*PI)*values[0])/std::pow(need_nonzero(values[1],"T"),2.0));
                              }else{
                                if(((calc_id=="M_SPRING"))){
                                  return (((values[0]*values[1])*values[1])/((4.0*PI)*PI));
                                }else{
                                  if(((calc_id=="OMEGA_SPRING"))){
                                    return need_sqrt((values[0]/need_nonzero(values[1],"m")));
                                  }else{
                                    if(((calc_id=="F_SPRING_OSC"))){
                                      return (need_sqrt((values[0]/need_nonzero(values[1],"m")))/(2.0*PI));
                                    }else{
                                      if(((calc_id=="T_PENDULUM"))){
                                        return ((2.0*PI)*need_sqrt((values[0]/need_nonzero(values[1],"g"))));
                                      }else{
                                        if(((calc_id=="L_PENDULUM"))){
                                          return (((values[0]*values[1])*values[1])/((4.0*PI)*PI));
                                        }else{
                                          if(((calc_id=="V_MAX"))){
                                            return (values[0]*values[1]);
                                          }else{
                                            if(((calc_id=="A_MAX"))){
                                              return ((values[0]*values[0])*values[1]);
                                            }else{
                                              if(((calc_id=="E_SPRING"))){
                                                return (((0.5*values[0])*values[1])*values[1]);
                                              }else{
                                                if(((calc_id=="A_FROM_VMAX_OMEGA"))){
                                                  return (values[0]/need_nonzero(values[1],"omega"));
                                                }else{
                                                  if(((calc_id=="A_FROM_AMAX_OMEGA"))){
                                                    auto omega=need_nonzero(values[1],"omega");
                                                    return (values[0]/(omega*omega));
                                                  }else{
                                                    if(((calc_id=="OMEGA_FROM_VMAX_A"))){
                                                      return (values[0]/need_nonzero(values[1],"A_s"));
                                                    }else{
                                                      if(((calc_id=="OMEGA_FROM_AMAX_A"))){
                                                        return need_sqrt((values[0]/need_nonzero(values[1],"A_s")));
                                                      }else{
                                                        if(((calc_id=="G_PENDULUM"))){
                                                          return ((((4.0*PI)*PI)*values[0])/std::pow(need_nonzero(values[1],"T"),2.0));
                                                        }else{
                                                          if(((calc_id=="OMEGA_LC"))){
                                                            return (1.0/need_sqrt(need_nonzero((values[0]*values[1]),"L*C")));
                                                          }else{
                                                            if(((calc_id=="F_LC"))){
                                                              return (1.0/((2.0*PI)*need_sqrt(need_nonzero((values[0]*values[1]),"L*C"))));
                                                            }else{
                                                              if(((calc_id=="T_LC"))){
                                                                return ((2.0*PI)*need_sqrt((values[0]*values[1])));
                                                              }else{
                                                                if(((calc_id=="L_FROM_TC"))){
                                                                  return ((values[0]*values[0])/(((4.0*PI)*PI)*need_nonzero(values[1],"C")));
                                                                }else{
                                                                  if(((calc_id=="C_FROM_TL"))){
                                                                    return ((values[0]*values[0])/(((4.0*PI)*PI)*need_nonzero(values[1],"L")));
                                                                  }else{
                                                                    if(((calc_id=="I0_FROM_U0"))){
                                                                      return (values[0]*need_sqrt((values[1]/need_nonzero(values[2],"L"))));
                                                                    }else{
                                                                      if(((calc_id=="U0_FROM_I0"))){
                                                                        return (values[0]*need_sqrt((values[1]/need_nonzero(values[2],"C"))));
                                                                      }else{
                                                                        if(((calc_id=="E_CAP_OSC"))){
                                                                          return (((0.5*values[0])*values[1])*values[1]);
                                                                        }else{
                                                                          if(((calc_id=="E_COIL_OSC"))){
                                                                            return (((0.5*values[0])*values[1])*values[1]);
                                                                          }else{
                                                                            if(((calc_id=="L_FROM_OMEGA_C"))){
                                                                              auto omega=need_nonzero(values[0],"omega0");
                                                                              return (1.0/((omega*omega)*need_nonzero(values[1],"C")));
                                                                            }else{
                                                                              if(((calc_id=="C_FROM_OMEGA_L"))){
                                                                                auto omega=need_nonzero(values[0],"omega0");
                                                                                return (1.0/((omega*omega)*need_nonzero(values[1],"L")));
                                                                              }else{
                                                                                if(((calc_id=="XL"))){
                                                                                  return (((2.0*PI)*values[0])*values[1]);
                                                                                }else{
                                                                                  if(((calc_id=="XC"))){
                                                                                    return (1.0/(((2.0*PI)*need_nonzero(values[0],"f"))*need_nonzero(values[1],"C")));
                                                                                  }else{
                                                                                    if(((calc_id=="Z_RLC"))){
                                                                                      return need_sqrt(((values[0]*values[0])+((values[1]-values[2])*(values[1]-values[2]))));
                                                                                    }else{
                                                                                      if(((calc_id=="I_EFF"))){
                                                                                        return (values[0]/need_nonzero(values[1],"Z"));
                                                                                      }else{
                                                                                        if(((calc_id=="P_AC"))){
                                                                                          return ((values[0]*values[1])*values[2]);
                                                                                        }else{
                                                                                          if(((calc_id=="COS_PHI"))){
                                                                                            return (values[0]/need_nonzero(values[1],"Z"));
                                                                                          }else{
                                                                                            if(((calc_id=="U_R_AC"))){
                                                                                              return (values[0]*values[1]);
                                                                                            }else{
                                                                                              if(((calc_id=="U_L_AC"))){
                                                                                                return (values[0]*values[1]);
                                                                                              }else{
                                                                                                if(((calc_id=="U_C_AC"))){
                                                                                                  return (values[0]*values[1]);
                                                                                                }else{
                                                                                                  if(((calc_id=="L_FROM_XL_F"))){
                                                                                                    return (values[0]/((2.0*PI)*need_nonzero(values[1],"f")));
                                                                                                  }else{
                                                                                                    if(((calc_id=="F_FROM_XL_L"))){
                                                                                                      return (values[0]/((2.0*PI)*need_nonzero(values[1],"L")));
                                                                                                    }else{
                                                                                                      if(((calc_id=="C_FROM_XC_F"))){
                                                                                                        return (1.0/(((2.0*PI)*need_nonzero(values[0],"X_C"))*need_nonzero(values[1],"f")));
                                                                                                      }else{
                                                                                                        if(((calc_id=="F_FROM_XC_C"))){
                                                                                                          return (1.0/(((2.0*PI)*need_nonzero(values[0],"X_C"))*need_nonzero(values[1],"C")));
                                                                                                        }else{
                                                                                                          if(((calc_id=="U_EFF_FROM_IZ"))){
                                                                                                            return (values[0]*values[1]);
                                                                                                          }else{
                                                                                                            if(((calc_id=="R_FROM_Z_COS"))){
                                                                                                              return (values[0]*values[1]);
                                                                                                            }else{
                                                                                                              if(((calc_id=="S_AC"))){
                                                                                                                return (values[0]*values[1]);
                                                                                                              }else{
                                                                                                                if(((calc_id=="Q_AC"))){
                                                                                                                  return ((values[0]*values[1])*values[2]);
                                                                                                                }else{
                                                                                                                  if(((calc_id=="P_FROM_S_COS"))){
                                                                                                                    return (values[0]*values[1]);
                                                                                                                  }else{
                                                                                                                    if(((calc_id=="Q_FROM_S_SIN"))){
                                                                                                                      return (values[0]*values[1]);
                                                                                                                    }else{
                                                                                                                      if(((calc_id=="S_FROM_P_Q"))){
                                                                                                                        return need_sqrt(((values[0]*values[0])+(values[1]*values[1])));
                                                                                                                      }else{
                                                                                                                        if(((calc_id=="COS_FROM_P_S"))){
                                                                                                                          return (values[0]/need_nonzero(values[1],"S"));
                                                                                                                        }else{
                                                                                                                          if(((calc_id=="SIN_FROM_Q_S"))){
                                                                                                                            return (values[0]/need_nonzero(values[1],"S"));
                                                                                                                          }else{
                                                                                                                            if(((calc_id=="Q_L"))){
                                                                                                                              return ((values[0]*values[0])*values[1]);
                                                                                                                            }else{
                                                                                                                              if(((calc_id=="Q_C"))){
                                                                                                                                return ((values[0]*values[0])*values[1]);
                                                                                                                              }else{
                                                                                                                                if(((calc_id=="U_OHM"))){
                                                                                                                                  return (values[0]*values[1]);
                                                                                                                                }else{
                                                                                                                                  if(((calc_id=="I_OHM"))){
                                                                                                                                    return (values[0]/need_nonzero(values[1],"R"));
                                                                                                                                  }else{
                                                                                                                                    if(((calc_id=="R_OHM"))){
                                                                                                                                      return (values[0]/need_nonzero(values[1],"I"));
                                                                                                                                    }else{
                                                                                                                                      if(((calc_id=="P_UI"))){
                                                                                                                                        return (values[0]*values[1]);
                                                                                                                                      }else{
                                                                                                                                        if(((calc_id=="Q_CU"))){
                                                                                                                                          return (values[0]*values[1]);
                                                                                                                                        }else{
                                                                                                                                          if(((calc_id=="C_QU"))){
                                                                                                                                            return (values[0]/need_nonzero(values[1],"U"));
                                                                                                                                          }else{
                                                                                                                                            if(((calc_id=="E_FIELD"))){
                                                                                                                                              return (values[0]/need_nonzero(values[1],"d"));
                                                                                                                                            }else{
                                                                                                                                              if(((calc_id=="F_QE"))){
                                                                                                                                                return (values[0]*values[1]);
                                                                                                                                              }else{
                                                                                                                                                if(((calc_id=="W_QU"))){
                                                                                                                                                  return (values[0]*values[1]);
                                                                                                                                                }else{
                                                                                                                                                  if(((calc_id=="C_WAVE"))){
                                                                                                                                                    return (values[0]*values[1]);
                                                                                                                                                  }else{
                                                                                                                                                    if(((calc_id=="F_WAVE"))){
                                                                                                                                                      return (values[0]/need_nonzero(values[1],"lambda"));
                                                                                                                                                    }else{
                                                                                                                                                      if(((calc_id=="LAMBDA_WAVE"))){
                                                                                                                                                        return (values[0]/need_nonzero(values[1],"f"));
                                                                                                                                                      }else{
                                                                                                                                                        if(((calc_id=="T_WAVE"))){
                                                                                                                                                          return (values[0]/need_nonzero(values[1],"c"));
                                                                                                                                                        }else{
                                                                                                                                                          if(((calc_id=="K_WAVE"))){
                                                                                                                                                            return ((2.0*PI)/need_nonzero(values[0],"lambda"));
                                                                                                                                                          }else{
                                                                                                                                                            if(((calc_id=="F_LORENTZ"))){
                                                                                                                                                              return ((values[0]*values[1])*values[2]);
                                                                                                                                                            }else{
                                                                                                                                                              if(((calc_id=="F_WIRE"))){
                                                                                                                                                                return ((values[0]*values[1])*values[2]);
                                                                                                                                                              }else{
                                                                                                                                                                if(((calc_id=="U_IND_BLV"))){
                                                                                                                                                                  return ((values[0]*values[1])*values[2]);
                                                                                                                                                                }else{
                                                                                                                                                                  if(((calc_id=="U_IND_NPHI"))){
                                                                                                                                                                    return ((values[0]*values[1])/need_nonzero(values[2],"dt"));
                                                                                                                                                                  }else{
                                                                                                                                                                    if(((calc_id=="B_WIRE"))){
                                                                                                                                                                      return (values[0]/need_nonzero((values[1]*values[2]),"I*l"));
                                                                                                                                                                    }
                                                                                                                                                                  }
                                                                                                                                                                }
                                                                                                                                                              }
                                                                                                                                                            }
                                                                                                                                                          }
                                                                                                                                                        }
                                                                                                                                                      }
                                                                                                                                                    }
                                                                                                                                                  }
                                                                                                                                                }
                                                                                                                                              }
                                                                                                                                            }
                                                                                                                                          }
                                                                                                                                        }
                                                                                                                                      }
                                                                                                                                    }
                                                                                                                                  }
                                                                                                                                }
                                                                                                                              }
                                                                                                                            }
                                                                                                                          }
                                                                                                                        }
                                                                                                                      }
                                                                                                                    }
                                                                                                                  }
                                                                                                                }
                                                                                                              }
                                                                                                            }
                                                                                                          }
                                                                                                        }
                                                                                                      }
                                                                                                    }
                                                                                                  }
                                                                                                }
                                                                                              }
                                                                                            }
                                                                                          }
                                                                                        }
                                                                                      }
                                                                                    }
                                                                                  }
                                                                                }
                                                                              }
                                                                            }
                                                                          }
                                                                        }
                                                                      }
                                                                    }
                                                                  }
                                                                }
                                                              }
                                                            }
                                                          }
                                                        }
                                                      }
                                                    }
                                                  }
                                                }
                                              }
                                            }
                                          }
                                        }
                                      }
                                    }
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  if((calc_id.rfind("Q_",0)==0)){
    return finite(quantum(calc_id,values));
  }
  return fail("Unbekannte Formel");
}
double quantum(const std::string& ident,const std::vector<double>& values){
  auto k=ident.substr(2);
  const auto& v=values;
  if((!(k=="LEVEL_D"||k=="LEVEL_L"||k=="LEVEL_F"))){
    for(double value:v){
      nonnegative(value);
    }
  }
  if(((k=="EF"))){
    return (H_PLANCK*positive(v[0]));
  }
  if(((k=="EL"))){
    return ((H_PLANCK*LIGHT_C)/positive(v[0]));
  }
  if(((k=="FE"))){
    return (positive(v[0])/H_PLANCK);
  }
  if(((k=="LE"))){
    return ((H_PLANCK*LIGHT_C)/positive(v[0]));
  }
  if(((k=="PL"||k=="LP"||k=="DB"||k=="PD"))){
    return (H_PLANCK/positive(v[0]));
  }
  if(((k=="PE"))){
    return (positive(v[0])/LIGHT_C);
  }
  if(((k=="EP"))){
    return (positive(v[0])*LIGHT_C);
  }
  if(((k=="NE"))){
    return (v[0]/positive(v[1]));
  }
  if(((k=="EN"))){
    return (v[0]*positive(v[1]));
  }
  if(((k=="NPT"))){
    return ((v[0]*v[1])/positive(v[2]));
  }
  if(((k=="PN"))){
    return ((v[0]*positive(v[1]))/positive(v[2]));
  }
  if(((k=="TN"))){
    return ((v[0]*positive(v[1]))/positive(v[2]));
  }
  if(((k=="KF"||k=="KL"))){
    auto energy=(((k=="KF"))?(H_PLANCK*positive(v[0])):((H_PLANCK*LIGHT_C)/positive(v[0])));
    auto result=(energy-v[1]);
    if(((result<0.0))){
      return fail("Kein Photoeffekt: unter Schwelle");
    }
    return result;
  }
  if(((k=="UK"||k=="EXU"))){
    return (v[0]/E_CHARGE);
  }
  if(((k=="KU"||k=="EU"||k=="EX"))){
    return (E_CHARGE*v[0]);
  }
  if(((k=="WF"||k=="WL"))){
    auto result=(((k=="WF"))?((H_PLANCK*v[0])-(E_CHARGE*v[1])):(((H_PLANCK*LIGHT_C)/positive(v[0]))-v[1]));
    if(((result<0.0))){
      return fail("Negative Austrittsarbeit: Daten pruefen");
    }
    return result;
  }
  if(((k=="FG"))){
    return (v[0]/H_PLANCK);
  }
  if(((k=="WFG"))){
    return (H_PLANCK*v[0]);
  }
  if(((k=="LG"||k=="WLG"))){
    return ((H_PLANCK*LIGHT_C)/positive(v[0]));
  }
  if(((k=="VK"))){
    return classical_v(need_sqrt(((2.0*v[0])/ELECTRON_MASS)));
  }
  if(((k=="FKW"))){
    return ((v[0]+v[1])/H_PLANCK);
  }
  if(((k=="LKW"))){
    return ((H_PLANCK*LIGHT_C)/positive((v[0]+v[1])));
  }
  if(((k=="LMV"))){
    return ((H_PLANCK/positive(v[0]))/positive(classical_v(v[1])));
  }
  if(((k=="VD"))){
    return classical_v(((H_PLANCK/positive(v[0]))/positive(v[1])));
  }
  if(((k=="MD"))){
    return ((H_PLANCK/positive(classical_v(v[0])))/positive(v[1]));
  }
  if(((k=="LK"))){
    classical_v(need_sqrt(((2.0*positive(v[1]))/positive(v[0]))));
    return ((H_PLANCK/need_sqrt((2.0*v[0])))/need_sqrt(v[1]));
  }
  if(((k=="KD"))){
    auto p=(H_PLANCK/positive(v[1]));
    classical_v((p/positive(v[0])));
    return (((p/v[0])*p)/2.0);
  }
  if(((k=="LUC"||k=="LUR"))){
    auto energy=(E_CHARGE*positive(v[0]));
    if(((k=="LUC"))){
      classical_v(need_sqrt(((2.0*energy)/ELECTRON_MASS)));
    }
    auto result=((H_PLANCK/need_sqrt((2.0*ELECTRON_MASS)))/need_sqrt(energy));
    if(((k=="LUR"))){
      result/=need_sqrt((1.0+(energy/(((2.0*ELECTRON_MASS)*LIGHT_C)*LIGHT_C))));
    }
    return result;
  }
  if(((k=="UD"))){
    auto speed=classical_v(((H_PLANCK/ELECTRON_MASS)/positive(v[0])));
    return ((((ELECTRON_MASS/E_CHARGE)*speed)*speed)/2.0);
  }
  if(((k=="BL"||k=="BD"||k=="BT"||k=="RL"||k=="RR"))){
    auto n=natural(v.back());
    if(((k=="BL"||k=="BD"))){
      if(!(((0.0<v[1])&&(v[1]<=90.0)))){
        return fail("Glanzwinkel: 0 < theta <= 90");
      }
      auto sine=std::sin(((v[1]*PI)/180.0));
      if(((k=="BL"))){
        return (((2.0*positive(v[0]))*sine)/n);
      }
      return ((n*positive(v[0]))/(2.0*sine));
    }
    if(((k=="BT"||k=="RR"))){
      auto ratio=((n*positive(v[0]))/(2.0*positive(v[1])));
      if(((ratio>1.0))){
        return fail("Bragg-Bedingung nicht erfuellbar");
      }
      auto angle=std::asin(ratio);
      if(((k=="BT"))){
        return ((angle*180.0)/PI);
      }
      if(((angle>=(PI/4.0)))){
        return fail("Ring trifft vorderen Schirm nicht");
      }
      return (positive(v[2])*std::tan((2.0*angle)));
    }
    return (((2.0*positive(v[0]))*std::sin((std::atan((positive(v[1])/positive(v[2])))/2.0)))/n);
  }
  if(((k=="DS"||k=="DL"||k=="DD"||k=="DDIST"))){
    for(double value:v){
      positive(value);
    }
    return ((v[0]*v[1])/v[2]);
  }
  if(((k=="SP"||k=="SX"))){
    return ((H_PLANCK/(4.0*PI))/positive(v[0]));
  }
  if(((k=="SV"||k=="SXV"))){
    return (((H_PLANCK/(4.0*PI))/positive(v[0]))/positive(v[1]));
  }
  if(((k=="SVE"))){
    return ((H_PLANCK/((4.0*PI)*ELECTRON_MASS))/positive(v[0]));
  }
  if(((k=="SF"||k=="ST"))){
    return (1.0/((4.0*PI)*positive(v[0])));
  }
  if(((k=="GF"||k=="GT"))){
    return (((2.0*std::log(2.0))/PI)/positive(v[0]));
  }
  if(((k=="SCHOOL"||k=="SCHOOLT"))){
    return (1.0/positive(v[0]));
  }
  if(((k=="GAMMA"||k=="TAU"))){
    return ((H_PLANCK/(2.0*PI))/positive(v[0]));
  }
  if(((k=="BOX_L"))){
    return ((2.0*positive(v[0]))/natural(v[1]));
  }
  if(((k=="BOX_P"))){
    return ((H_PLANCK*natural(v[1]))/(2.0*positive(v[0])));
  }
  if(((k=="BOX_E"))){
    return box_energy(v[0],v[1]);
  }
  if(((k=="BOX_EM"))){
    return box_energy(v[0],v[1],v[2]);
  }
  if(((k=="BOX_A"))){
    return (((H_PLANCK*natural(v[1]))/need_sqrt((8.0*ELECTRON_MASS)))/need_sqrt(positive(v[0])));
  }
  if(((k=="BOX_D"||k=="BOX_PH"))){
    auto delta=std::abs((box_energy(v[0],v[1])-box_energy(v[0],v[2])));
    positive(delta);
    return (((k=="BOX_D"))?delta:((H_PLANCK*LIGHT_C)/delta));
  }
  if(((k=="BOHR"||k=="ION"||k=="RAD"||k=="H_D"||k=="H_PH"))){
    auto z=natural(v[0]);
    auto n=natural(v[1]);
    if(((k=="RAD"))){
      return (((BOHR_A*n)*n)/z);
    }
    auto energy=((13.6*E_CHARGE)*std::pow((z/n),2.0));
    if(((k=="BOHR"))){
      return -(energy);
    }
    if(((k=="ION"))){
      return energy;
    }
    auto nf=natural(v[2]);
    auto delta=std::abs((energy-((13.6*E_CHARGE)*std::pow((z/nf),2.0))));
    positive(delta);
    return (((k=="H_D"))?delta:((H_PLANCK*LIGHT_C)/delta));
  }
  if(((k=="LEVEL_D"||k=="LEVEL_L"||k=="LEVEL_F"))){
    for(double value:v){
      finite(value);
    }
    auto delta=positive(std::abs((v[0]-v[1])));
    if(((k=="LEVEL_D"))){
      return delta;
    }
    if(((k=="LEVEL_L"))){
      return ((H_PLANCK*LIGHT_C)/delta);
    }
    return (delta/H_PLANCK);
  }
  if(((k=="EXL"||k=="XR"||k=="XU"))){
    return (((H_PLANCK*LIGHT_C)/E_CHARGE)/positive(v[0]));
  }
  if(((k=="COMP"||k=="COMPL"))){
    auto theta=v.back();
    if(((theta>180.0))){
      return fail("Streuwinkel: 0 bis 180 Grad");
    }
    auto delta=(((H_PLANCK/ELECTRON_MASS)/LIGHT_C)*(1.0-std::cos(((theta*PI)/180.0))));
    return (((k=="COMP"))?delta:(positive(v[0])+delta));
  }
  return fail("Unbekannte Quantenformel");
}
int expectedCount(const std::string& id){
  if(id=="T_FROM_F")return 1;
  if(id=="F_FROM_T")return 1;
  if(id=="OMEGA_FROM_F")return 1;
  if(id=="F_FROM_OMEGA")return 1;
  if(id=="T_FROM_OMEGA")return 1;
  if(id=="V_FROM_S_T")return 2;
  if(id=="S_FROM_V_T")return 2;
  if(id=="A_FROM_DV_DT")return 2;
  if(id=="F_FROM_M_A")return 2;
  if(id=="P_FROM_M_V")return 2;
  if(id=="E_KIN")return 2;
  if(id=="E_POT")return 3;
  if(id=="P_MECH")return 2;
  if(id=="T_SPRING")return 2;
  if(id=="D_SPRING")return 2;
  if(id=="M_SPRING")return 2;
  if(id=="OMEGA_SPRING")return 2;
  if(id=="F_SPRING_OSC")return 2;
  if(id=="T_PENDULUM")return 2;
  if(id=="L_PENDULUM")return 2;
  if(id=="V_MAX")return 2;
  if(id=="A_MAX")return 2;
  if(id=="E_SPRING")return 2;
  if(id=="A_FROM_VMAX_OMEGA")return 2;
  if(id=="A_FROM_AMAX_OMEGA")return 2;
  if(id=="OMEGA_FROM_VMAX_A")return 2;
  if(id=="OMEGA_FROM_AMAX_A")return 2;
  if(id=="G_PENDULUM")return 2;
  if(id=="OMEGA_LC")return 2;
  if(id=="F_LC")return 2;
  if(id=="T_LC")return 2;
  if(id=="L_FROM_TC")return 2;
  if(id=="C_FROM_TL")return 2;
  if(id=="I0_FROM_U0")return 3;
  if(id=="U0_FROM_I0")return 3;
  if(id=="E_CAP_OSC")return 2;
  if(id=="E_COIL_OSC")return 2;
  if(id=="L_FROM_OMEGA_C")return 2;
  if(id=="C_FROM_OMEGA_L")return 2;
  if(id=="XL")return 2;
  if(id=="XC")return 2;
  if(id=="Z_RLC")return 3;
  if(id=="I_EFF")return 2;
  if(id=="P_AC")return 3;
  if(id=="COS_PHI")return 2;
  if(id=="U_R_AC")return 2;
  if(id=="U_L_AC")return 2;
  if(id=="U_C_AC")return 2;
  if(id=="L_FROM_XL_F")return 2;
  if(id=="F_FROM_XL_L")return 2;
  if(id=="C_FROM_XC_F")return 2;
  if(id=="F_FROM_XC_C")return 2;
  if(id=="U_EFF_FROM_IZ")return 2;
  if(id=="R_FROM_Z_COS")return 2;
  if(id=="S_AC")return 2;
  if(id=="Q_AC")return 3;
  if(id=="P_FROM_S_COS")return 2;
  if(id=="Q_FROM_S_SIN")return 2;
  if(id=="S_FROM_P_Q")return 2;
  if(id=="COS_FROM_P_S")return 2;
  if(id=="SIN_FROM_Q_S")return 2;
  if(id=="Q_L")return 2;
  if(id=="Q_C")return 2;
  if(id=="U_OHM")return 2;
  if(id=="I_OHM")return 2;
  if(id=="R_OHM")return 2;
  if(id=="P_UI")return 2;
  if(id=="Q_CU")return 2;
  if(id=="C_QU")return 2;
  if(id=="E_FIELD")return 2;
  if(id=="F_QE")return 2;
  if(id=="W_QU")return 2;
  if(id=="C_WAVE")return 2;
  if(id=="F_WAVE")return 2;
  if(id=="LAMBDA_WAVE")return 2;
  if(id=="T_WAVE")return 2;
  if(id=="K_WAVE")return 1;
  if(id=="F_LORENTZ")return 3;
  if(id=="F_WIRE")return 3;
  if(id=="U_IND_BLV")return 3;
  if(id=="U_IND_NPHI")return 3;
  if(id=="B_WIRE")return 3;
  if(id=="Q_EF")return 1;
  if(id=="Q_EL")return 1;
  if(id=="Q_FE")return 1;
  if(id=="Q_LE")return 1;
  if(id=="Q_PL")return 1;
  if(id=="Q_PE")return 1;
  if(id=="Q_EP")return 1;
  if(id=="Q_LP")return 1;
  if(id=="Q_NE")return 2;
  if(id=="Q_EN")return 2;
  if(id=="Q_NPT")return 3;
  if(id=="Q_PN")return 3;
  if(id=="Q_TN")return 3;
  if(id=="Q_KF")return 2;
  if(id=="Q_KL")return 2;
  if(id=="Q_UK")return 1;
  if(id=="Q_KU")return 1;
  if(id=="Q_WF")return 2;
  if(id=="Q_WL")return 2;
  if(id=="Q_FG")return 1;
  if(id=="Q_LG")return 1;
  if(id=="Q_WFG")return 1;
  if(id=="Q_WLG")return 1;
  if(id=="Q_VK")return 1;
  if(id=="Q_FKW")return 2;
  if(id=="Q_LKW")return 2;
  if(id=="Q_DB")return 1;
  if(id=="Q_PD")return 1;
  if(id=="Q_LMV")return 2;
  if(id=="Q_VD")return 2;
  if(id=="Q_MD")return 2;
  if(id=="Q_LK")return 2;
  if(id=="Q_KD")return 2;
  if(id=="Q_EU")return 1;
  if(id=="Q_LUC")return 1;
  if(id=="Q_LUR")return 1;
  if(id=="Q_UD")return 1;
  if(id=="Q_BL")return 3;
  if(id=="Q_BD")return 3;
  if(id=="Q_BT")return 3;
  if(id=="Q_RL")return 4;
  if(id=="Q_RR")return 4;
  if(id=="Q_DS")return 3;
  if(id=="Q_DL")return 3;
  if(id=="Q_DD")return 3;
  if(id=="Q_DDIST")return 3;
  if(id=="Q_SP")return 1;
  if(id=="Q_SX")return 1;
  if(id=="Q_SV")return 2;
  if(id=="Q_SVE")return 1;
  if(id=="Q_SXV")return 2;
  if(id=="Q_SF")return 1;
  if(id=="Q_ST")return 1;
  if(id=="Q_GF")return 1;
  if(id=="Q_GT")return 1;
  if(id=="Q_SCHOOL")return 1;
  if(id=="Q_SCHOOLT")return 1;
  if(id=="Q_GAMMA")return 1;
  if(id=="Q_TAU")return 1;
  if(id=="Q_BOX_L")return 2;
  if(id=="Q_BOX_P")return 2;
  if(id=="Q_BOX_E")return 2;
  if(id=="Q_BOX_EM")return 3;
  if(id=="Q_BOX_A")return 2;
  if(id=="Q_BOX_D")return 3;
  if(id=="Q_BOX_PH")return 3;
  if(id=="Q_BOHR")return 2;
  if(id=="Q_ION")return 2;
  if(id=="Q_RAD")return 2;
  if(id=="Q_H_D")return 3;
  if(id=="Q_H_PH")return 3;
  if(id=="Q_LEVEL_D")return 2;
  if(id=="Q_LEVEL_L")return 2;
  if(id=="Q_LEVEL_F")return 2;
  if(id=="Q_EX")return 1;
  if(id=="Q_EXL")return 1;
  if(id=="Q_EXU")return 1;
  if(id=="Q_XR")return 1;
  if(id=="Q_XU")return 1;
  if(id=="Q_COMP")return 1;
  if(id=="Q_COMPL")return 2;
  return -1;
}
