class ParticleManager{
  ArrayList<Particle> mParticles;
  
  int updateType;
  
  
  ParticleManager(){
    mParticles = new  ArrayList<Particle>();
  }
  
  void add(Particle p){
   mParticles.add(p);
  }
  
  void draw(PGraphics2D pg){
    for(Particle p : mParticles){
     p.update(pg); 
    }
    
  }
  
  void update(){
    
  }
  
}
