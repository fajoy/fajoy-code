class Hello{
   public native void hello();

   static {
      System.loadLibrary("hello");
   }
public static void main(String[] args){
   System.out.println("java hello.");
   new Hello().hello();
}

}
