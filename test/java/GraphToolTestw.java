import static org.junit.jupiter.api.Assertions.*; // 引入断言方法 assertEquals、assertTrue 等

import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.Test;                 // @Test 注解

import java.io.IOException;
import java.util.List;
public class GraphToolTestw {
  static GraphTool.Graph graph;
  @BeforeAll
  static void setup() throws IOException {
    graph = GraphTool.buildGraph("src/Easy Test.txt");
  }
  @Test
  void testShortestPath_normalCase() {
    List<String> path = graph.shortestPath("scientist", "team");
    List<String> expected = List.of("scientist", "analyzed", "the", "team");
    assertEquals(expected, path);
  }


  @Test
  void testShortestPath_targetNotExists() {
    List<String> path = graph.shortestPath("scientist", "unknown");
    assertTrue(path.isEmpty(), "目标不存在应返回空路径");
  }

  @Test
  void testShortestPath_sameStartEnd() {
    List<String> path = graph.shortestPath("scientist", "scientist");
    assertEquals(List.of("scientist"), path, "起点=终点应只包含自己");
  }

}